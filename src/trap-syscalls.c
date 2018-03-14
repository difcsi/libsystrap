#define _GNU_SOURCE
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
// #include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <link.h>
#include <maps.h> /* from liballocs (or librunt?) */
#include "systrap.h"
#include "systrap_private.h"
#include "syscall-names.h"
/* #include <footprints.h> */

/* We are a preloaded library whose constructor
 * calls libsystrap to divert all syscalls
 * into the generic syscall emulation path; we provide
 * pre_handler and post_handler to print stuff. */

_Bool __write_traces;
void *traces_out __attribute__((visibility("hidden"))) /* really FILE* */;
int trace_fd __attribute__((visibility("hidden")));

static int process_mapping_cb(struct maps_entry *ent, char *linebuf, void *arg)
{
	/* Skip ourselves, but remember our load address. */
	void *expected_mapping_end = (void*) page_boundary_up((uintptr_t) &etext);
	if ((const unsigned char *) ent->second >= (const unsigned char *) expected_mapping_end
		 && (const unsigned char *) ent->first < (const unsigned char *) expected_mapping_end)
	{
		our_text_begin_address = (const void *) ent->first;
		our_text_end_address = (const void *) ent->second;
		
		/* Compute our load address from the phdr p_vaddr of this segment.
		 * But how do we get at our phdrs?
		 * In general I think we need to hack the linker script to define a new symbol.
		 * But for now, just use the fact that it's very likely to be the lowest text addr. */
		our_load_address = (uintptr_t) our_text_begin_address;

		debug_printf(1, "Skipping our own text mapping: %p-%p\n", 
			(void*) ent->first, (void*) ent->second);
		
		return 0; // keep going
	}

	if (ent->x == 'x')
	{
		trap_one_executable_region((unsigned char *) ent->first, (unsigned char *) ent->second,
			 ent->rest[0] ? ent->rest : NULL,
			ent->w == 'w', ent->r == 'r');
	}
	
	return 0; // keep going
}

void trap_all_mappings(void)
{
	/* When we process mappings, we do mprotect()s, which can change the memory map, 
	 * including removing/adding lines. So there's a race condition unless we eagerly
	 * snapshot the map. Do that here. */
	int fd = raw_open("/proc/self/maps", O_RDONLY);
	if (fd != -1)
	{
		/* We run during startup, so the number of distinct /proc lines should be small. */
	#define MAX_LINES 1024
		char *lines[MAX_LINES];
		int n = 0;
		ssize_t linesz;
		char linebuf[8192];
		while (-1 != (linesz = get_a_line_from_maps_fd(linebuf, sizeof linebuf, fd)))
		{
			lines[n] = alloca(linesz + 1);
			if (!lines[n]) abort();
			strncpy(lines[n], linebuf, linesz);
			lines[n][linesz] = '\0';
			++n;
		}
		
		/* Now we have an array containing the lines. */
		struct maps_entry entry;
		for (int i = 0; i < n; ++i)
		{
			int ret = process_one_maps_entry(lines[i], &entry, process_mapping_cb, NULL);
			if (ret) break;
		}

		raw_close(fd);
	}
}

static void startup(void) __attribute__((constructor(101)));
static void startup(void)
{
	if (getenv("TRAP_SYSCALLS_DELAY_STARTUP"))
	{
		sleep(atoi(getenv("TRAP_SYSCALLS_DELAY_STARTUP")));
	}
	
	char *trace_fd_str = getenv("TRAP_SYSCALLS_TRACE_FD");
	if (trace_fd_str) trace_fd = atoi(trace_fd_str);

	debug_printf(0, "TRAP_SYSCALLS_TRACE_FD is %s, ", trace_fd_str);
	if (!trace_fd_str || trace_fd == 2)
	{
		debug_printf(0, "dup'ing stderr, ");
		trace_fd = dup(2);
	}
	
	if (trace_fd >= 0)
	{
		if (fcntl(F_GETFD, trace_fd) != -1)
		{
			debug_printf(0, "fd %d is open; outputting traces there.\n", trace_fd);
			__write_traces = 1;
			traces_out = fdopen(trace_fd, "a");
			if (!traces_out)
			{
				debug_printf(0, "Could not open traces output stream for writing!\n");
			}
		}
		else
		{
			debug_printf(0, "fd %d is closed; not outputting traces.\n", trace_fd);
		}
	}
	else debug_printf(0, "not outputting traces.\n");
	
	trap_all_mappings();
	install_sigill_handler();
}

void print_pre_syscall(void *stream, struct generic_syscall *gsp, void *calling_addr, struct link_map *calling_object, void *ret)
			__attribute__((visibility("protected")));
void print_pre_syscall(void *stream, struct generic_syscall *gsp, void *calling_addr, struct link_map *calling_object, void *ret) {
	fprintf(stream, "== %d == > %p (%s+0x%x) %s(%p, %p, %p, %p, %p, %p)\n",
			 getpid(), 
			 calling_addr,
			 calling_object->l_name,
			 (char*) calling_addr - (char*) calling_object->l_addr,
			 syscall_names[gsp->syscall_number],
			 gsp->args[0],
			 gsp->args[1],
			 gsp->args[2],
			 gsp->args[3],
			 gsp->args[4],
			 gsp->args[5]
		);
	fflush(stream);
}

void print_post_syscall(void *stream, struct generic_syscall *gsp, void *calling_addr, struct link_map *calling_object, void *ret)
			__attribute__((visibility("protected")));
void print_post_syscall(void *stream, struct generic_syscall *gsp, void *calling_addr, struct link_map *calling_object, void *ret) {
	fprintf(stream, "== %d == < %p (%s+0x%x) %s(%p, %p, %p, %p, %p, %p) = %p\n",
		getpid(),
		calling_addr,
		calling_object->l_name,
		(char*) calling_addr - (char*) calling_object->l_addr,
		syscall_names[gsp->syscall_number],
			gsp->args[0],
			gsp->args[1],
			gsp->args[2],
			gsp->args[3],
			gsp->args[4],
			gsp->args[5],
		ret
	);
	fflush(stream);
}
