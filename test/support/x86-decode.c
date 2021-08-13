#include "x86_defs.h"
#include <udis86.h> // for comparison against udis86
#include <string.h>

_Bool any_failed;

struct cpu_user_regs regs;
struct x86_emulate_ctxt ctxt = {
	.addr_size = 64,
	.sp_size = 64
};
static int insn_fetch(
        enum x86_segment seg,
        unsigned long offset,
        void *p_data,
        unsigned int bytes,
        struct x86_emulate_ctxt *ctxt)
{
        memcpy(p_data, (void*) offset, bytes);
        return X86EMUL_OKAY;
}
static struct x86_emulate_ops ops = {
        .insn_fetch = insn_fetch
};

unsigned long
instr_len(unsigned const char *ins)
{
	if (!ctxt.regs) ctxt.regs = &regs;
	ctxt.regs->rip = (uintptr_t) ins;
	int x86_decode_len = x86_decode(&ctxt, &ops);
	int x86_decode_err = (x86_decode_len > 0) ? 0 : -x86_decode_len;
	if (x86_decode_len < 1)
	{
		any_failed = 1;
		warnx("x86_decode failed on instruction at %p (err %d)", ins, x86_decode_err);
	}
	return x86_decode_len;
}

struct instr_test {
	int len;
	char *instr_bytes;
} tests[] = {
		{ 10, "\xc4\x62\x79\x29\x84\x24\xc0\x02\x00\x00" /* vpcmpeqq 0x\x2c0(%rsp),%xmm0,%xmm8*/ }, //      1
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //      2
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //      3
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x17\x21*/ }, //      4
		{ 9, "\xc5\xf9\x7f\x84\x24\xc0\x00\x00\x00" /* vmovdqa %xmm0,0x\xc0(%rsp)*/ }, //      5
		{ 2, "\xeb\x0f" /* jmp    0x\x2a\xaa\xaa\xac\x17\x30*/ }, //      6
		{ 9, "\xc5\xfe\x6f\x84\x24\xc0\x00\x00\x00" /* vmovdqu 0x\xc0(%rsp),%ymm0*/ }, //      7
		{ 6, "\xc5\xf9\x7f\x44\x24\x40" /* vmovdqa %xmm0,0x\x40(%rsp)*/ }, //      8
		{ 10, "\xc4\x62\x71\x29\x84\x24\xd0\x02\x00\x00" /* vpcmpeqq 0x\x2d0(%rsp),%xmm1,%xmm8*/ }, //      9
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     10
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     11
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x17\x52*/ }, //     12
		{ 9, "\xc5\xf9\x7f\x8c\x24\x00\x01\x00\x00" /* vmovdqa %xmm1,0x\x100(%rsp)*/ }, //     13
		{ 2, "\xeb\x0f" /* jmp    0x\x2a\xaa\xaa\xac\x17\x61*/ }, //     14
		{ 9, "\xc5\xfe\x6f\x8c\x24\x00\x01\x00\x00" /* vmovdqu 0x\x100(%rsp),%ymm1*/ }, //     15
		{ 6, "\xc5\xf9\x7f\x4c\x24\x50" /* vmovdqa %xmm1,0x\x50(%rsp)*/ }, //     16
		{ 10, "\xc4\x62\x69\x29\x84\x24\xe0\x02\x00\x00" /* vpcmpeqq 0x\x2e0(%rsp),%xmm2,%xmm8*/ }, //     17
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     18
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     19
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x17\x83*/ }, //     20
		{ 9, "\xc5\xf9\x7f\x94\x24\x40\x01\x00\x00" /* vmovdqa %xmm2,0x\x140(%rsp)*/ }, //     21
		{ 2, "\xeb\x0f" /* jmp    0x\x2a\xaa\xaa\xac\x17\x92*/ }, //     22
		{ 9, "\xc5\xfe\x6f\x94\x24\x40\x01\x00\x00" /* vmovdqu 0x\x140(%rsp),%ymm2*/ }, //     23
		{ 6, "\xc5\xf9\x7f\x54\x24\x60" /* vmovdqa %xmm2,0x\x60(%rsp)*/ }, //     24
		{ 10, "\xc4\x62\x61\x29\x84\x24\xf0\x02\x00\x00" /* vpcmpeqq 0x\x2f0(%rsp),%xmm3,%xmm8*/ }, //     25
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     26
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     27
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x17\xb4*/ }, //     28
		{ 9, "\xc5\xf9\x7f\x9c\x24\x80\x01\x00\x00" /* vmovdqa %xmm3,0x\x180(%rsp)*/ }, //     29
		{ 2, "\xeb\x0f" /* jmp    0x\x2a\xaa\xaa\xac\x17\xc3*/ }, //     30
		{ 9, "\xc5\xfe\x6f\x9c\x24\x80\x01\x00\x00" /* vmovdqu 0x\x180(%rsp),%ymm3*/ }, //     31
		{ 6, "\xc5\xf9\x7f\x5c\x24\x70" /* vmovdqa %xmm3,0x\x70(%rsp)*/ }, //     32
		{ 10, "\xc4\x62\x59\x29\x84\x24\x00\x03\x00\x00" /* vpcmpeqq 0x\x300(%rsp),%xmm4,%xmm8*/ }, //     33
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     34
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     35
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x17\xe5*/ }, //     36
		{ 9, "\xc5\xf9\x7f\xa4\x24\xc0\x01\x00\x00" /* vmovdqa %xmm4,0x\x1c0(%rsp)*/ }, //     37
		{ 2, "\xeb\x12" /* jmp    0x\x2a\xaa\xaa\xac\x17\xf7*/ }, //     38
		{ 9, "\xc5\xfe\x6f\xa4\x24\xc0\x01\x00\x00" /* vmovdqu 0x\x1c0(%rsp),%ymm4*/ }, //     39
		{ 9, "\xc5\xf9\x7f\xa4\x24\x80\x00\x00\x00" /* vmovdqa %xmm4,0x\x80(%rsp)*/ }, //     40
		{ 10, "\xc4\x62\x51\x29\x84\x24\x10\x03\x00\x00" /* vpcmpeqq 0x\x310(%rsp),%xmm5,%xmm8*/ }, //     41
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     42
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     43
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x18\x19*/ }, //     44
		{ 9, "\xc5\xf9\x7f\xac\x24\x00\x02\x00\x00" /* vmovdqa %xmm5,0x\x200(%rsp)*/ }, //     45
		{ 2, "\xeb\x12" /* jmp    0x\x2a\xaa\xaa\xac\x18\x2b*/ }, //     46
		{ 9, "\xc5\xfe\x6f\xac\x24\x00\x02\x00\x00" /* vmovdqu 0x\x200(%rsp),%ymm5*/ }, //     47
		{ 9, "\xc5\xf9\x7f\xac\x24\x90\x00\x00\x00" /* vmovdqa %xmm5,0x\x90(%rsp)*/ }, //     48
		{ 10, "\xc4\x62\x49\x29\x84\x24\x20\x03\x00\x00" /* vpcmpeqq 0x\x320(%rsp),%xmm6,%xmm8*/ }, //     49
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     50
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     51
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x18\x4d*/ }, //     52
		{ 9, "\xc5\xf9\x7f\xb4\x24\x40\x02\x00\x00" /* vmovdqa %xmm6,0x\x240(%rsp)*/ }, //     53
		{ 2, "\xeb\x12" /* jmp    0x\x2a\xaa\xaa\xac\x18\x5f*/ }, //     54
		{ 9, "\xc5\xfe\x6f\xb4\x24\x40\x02\x00\x00" /* vmovdqu 0x\x240(%rsp),%ymm6*/ }, //     55
		{ 9, "\xc5\xf9\x7f\xb4\x24\xa0\x00\x00\x00" /* vmovdqa %xmm6,0x\xa0(%rsp)*/ }, //     56
		{ 10, "\xc4\x62\x41\x29\x84\x24\x30\x03\x00\x00" /* vpcmpeqq 0x\x330(%rsp),%xmm7,%xmm8*/ }, //     57
		{ 5, "\xc4\xc1\x79\xd7\xf0" /* vpmovmskb %xmm8,%esi*/ }, //     58
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //     59
		{ 2, "\x74\x0b" /* je     0x\x2a\xaa\xaa\xac\x18\x81*/ }, //     60
		{ 9, "\xc5\xf9\x7f\xbc\x24\x80\x02\x00\x00" /* vmovdqa %xmm7,0x\x280(%rsp)*/ }, //     61
		{ 2, "\xeb\x12" /* jmp    0x\x2a\xaa\xaa\xac\x18\x93*/ }, //     62
		{ 9, "\xc5\xfe\x6f\xbc\x24\x80\x02\x00\x00" /* vmovdqu 0x\x280(%rsp),%ymm7*/ }, //     63
		{ 9, "\xc5\xf9\x7f\xbc\x24\xb0\x00\x00\x00" /* vmovdqa %xmm7,0x\xb0(%rsp)*/ }, //     64
		{ 4, "\x4c\x8b\x53\x10" /* mov    0x\x10(%rbx),%r\x10*/ }, //     65
		{ 3, "\x4d\x85\xd2" /* test   %r\x10,%r\x10*/ }, //     66
		{ 2, "\x79\x1d" /* jns    0x\x2a\xaa\xaa\xac\x18\xb9*/ }, //     67
		{ 5, "\x48\x8b\x4c\x24\x18" /* mov    0x\x18(%rsp),%rcx*/ }, //     68
		{ 5, "\x48\x8b\x74\x24\x20" /* mov    0x\x20(%rsp),%rsi*/ }, //     69
		{ 5, "\x48\x8b\x7c\x24\x28" /* mov    0x\x28(%rsp),%rdi*/ }, //     70
		{ 3, "\x48\x89\xdc" /* mov    %rbx,%rsp*/ }, //     71
		{ 4, "\x48\x8b\x1c\x24" /* mov    (%rsp),%rbx*/ }, //     72
		{ 4, "\x48\x83\xc4\x30" /* \xadd    $0x\x30,%rsp*/ }, //     73
		{ 3, "\x41\xff\xe3" /* jmpq   *%r\x11*/ }, //     74
		{ 4, "\x48\x8d\x73\x38" /* l\xea   0x\x38(%rbx),%rsi*/ }, //     75
		{ 4, "\x49\x83\xc2\x08" /* \xadd    $0x8,%r\x10*/ }, //     76
		{ 4, "\x49\x83\xe2\xf0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xf0,%r\x10*/ }, //     77
		{ 3, "\x4c\x89\xd1" /* mov    %r\x10,%rcx*/ }, //     78
		{ 3, "\x4c\x29\xd4" /* sub    %r\x10,%rsp*/ }, //     79
		{ 3, "\x48\x89\xe7" /* mov    %rsp,%rdi*/ }, //     80
		{ 4, "\x48\xc1\xe9\x03" /* shr    $0x3,%rcx*/ }, //     81
		{ 3, "\xf3\x48\xa5" /* rep movsq %ds:(%rsi),%es:(%rdi)*/ }, //     82
		{ 4, "\x48\x8b\x4f\x18" /* mov    0x\x18(%rdi),%rcx*/ }, //     83
		{ 4, "\x48\x8b\x77\x20" /* mov    0x\x20(%rdi),%rsi*/ }, //     84
		{ 4, "\x48\x8b\x7f\x28" /* mov    0x\x28(%rdi),%rdi*/ }, //     85
		{ 3, "\x41\xff\xd3" /* \xcallq  *%r\x11*/ }, //     86
		{ 4, "\x48\x8b\x63\x18" /* mov    0x\x18(%rbx),%rsp*/ }, //     87
		{ 7, "\x48\x81\xec\xf0\x00\x00\x00" /* sub    $0x\xf0,%rsp*/ }, //     88
		{ 3, "\x48\x89\xe1" /* mov    %rsp,%rcx*/ }, //     89
		{ 3, "\x48\x89\x01" /* mov    %rax,(%rcx)*/ }, //     90
		{ 4, "\x48\x89\x51\x08" /* mov    %rdx,0x8(%rcx)*/ }, //     91
		{ 4, "\x0f\x29\x41\x10" /* movaps %xmm0,0x\x10(%rcx)*/ }, //     92
		{ 4, "\x0f\x29\x49\x20" /* movaps %xmm1,0x\x20(%rcx)*/ }, //     93
		{ 5, "\xc5\xfe\x7f\x41\x50" /* vmovdqu %ymm0,0x\x50(%rcx)*/ }, //     94
		{ 8, "\xc5\xfe\x7f\x89\x90\x00\x00\x00" /* vmovdqu %ymm1,0x\x90(%rcx)*/ }, //     95
		{ 8, "\xc5\xf9\x7f\x81\xd0\x00\x00\x00" /* vmovdqa %xmm0,0x\xd0(%rcx)*/ }, //     96
		{ 8, "\xc5\xf9\x7f\x89\xe0\x00\x00\x00" /* vmovdqa %xmm1,0x\xe0(%rcx)*/ }, //     97
		{ 3, "\xdb\x79\x30" /* fstpt  0x\x30(%rcx)*/ }, //     98
		{ 3, "\xdb\x79\x40" /* fstpt  0x\x40(%rcx)*/ }, //     99
		{ 4, "\x48\x8b\x53\x18" /* mov    0x\x18(%rbx),%rdx*/ }, //    100
		{ 4, "\x48\x8b\x73\x28" /* mov    0x\x28(%rbx),%rsi*/ }, //    101
		{ 4, "\x48\x8b\x7b\x20" /* mov    0x\x20(%rbx),%rdi*/ }, //    102
		{ 5, "\xe8\x1b\x93\xff\xff" /* \xcallq  0x\x2a\xaa\xaa\xab\xac\x50*/ }, //    103
		{ 4, "\x48\x8b\x04\x24" /* mov    (%rsp),%rax*/ }, //    104
		{ 5, "\x48\x8b\x54\x24\x08" /* mov    0x8(%rsp),%rdx*/ }, //    105
		{ 5, "\x0f\x28\x44\x24\x10" /* movaps 0x\x10(%rsp),%xmm0*/ }, //    106
		{ 5, "\x0f\x28\x4c\x24\x20" /* movaps 0x\x20(%rsp),%xmm1*/ }, //    107
		{ 10, "\xc4\xe2\x79\x29\x94\x24\xd0\x00\x00\x00" /* vpcmpeqq 0x\xd0(%rsp),%xmm0,%xmm2*/ }, //    108
		{ 4, "\xc5\xf9\xd7\xf2" /* vpmovmskb %xmm2,%esi*/ }, //    109
		{ 6, "\x81\xfe\xff\xff\x00\x00" /* cmp    $0x\xff\xff,%esi*/ }, //    110
		{ 2, "\x75\x06" /* jne    0x\x2a\xaa\xaa\xac\x19\x64*/ }, //    111
		{ 6, "\xc5\xfe\x6f\x44\x24\x50" /* vmovdqu 0x\x50(%rsp),%ymm0*/ }, //    112
		{ 10, "\xc4\xe2\x71\x29\x94\x24\xe0\x00\x00\x00" /* vpcmpeqq 0x\xe0(%rsp),%xmm1,%xmm2*/ }, //    113
		{ 4, "\xc5\xf9\xd7\xf2" /* vpmovmskb %xmm2,%esi*/ }, //    114
		{ 4, "\x66\x0f\x6e\xce" /* movd   %esi,%xmm1*/ }, //    115
		{ 2, "\x89\xf8" /* mov    %\xedi,%\xeax*/ }, //    116
		{ 5, "\x25\xff\x0f\x00\x00" /* and    $0x\xfff,%\xeax*/ }, //    117
		{ 4, "\x66\x0f\x60\xc9" /* punpcklbw %xmm1,%xmm1*/ }, //    118
		{ 5, "\x3d\xc0\x0f\x00\x00" /* cmp    $0x\xfc0,%\xeax*/ }, //    119
		{ 4, "\x66\x0f\x61\xc9" /* punpcklwd %xmm1,%xmm1*/ }, //    120
		{ 5, "\x66\x0f\x70\xc9\x00" /* pshufd$0x0,%xmm1,%xmm1*/ }, //    121
		{ 6, "\x0f\x8f\x5d\x01\x00\x00" /* jg     0x\x2a\xaa\xaa\xac\x4b\xd0*/ }, //    122
		{ 4, "\xf3\x0f\x6f\x07" /* movdqu (%rdi),%xmm0*/ }, //    123
		{ 4, "\x66\x0f\xef\xdb" /* pxor   %xmm3,%xmm3*/ }, //    124
		{ 4, "\x66\x0f\x6f\xe0" /* movdqa %xmm0,%xmm4*/ }, //    125
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    126
		{ 4, "\x66\x0f\x74\xe3" /* pcmpeqb %xmm3,%xmm4*/ }, //    127
		{ 4, "\x66\x0f\xeb\xc4" /* por    %xmm4,%xmm0*/ }, //    128
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    129
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    130
		{ 2, "\x74\x15" /* je     0x\x2a\xaa\xaa\xac\x4a\xa8*/ }, //    131
		{ 3, "\x0f\xbc\xc0" /* bsf    %\xeax,%\xeax*/ }, //    132
		{ 5, "\xba\x00\x00\x00\x00" /* mov    $0x0,%\xedx*/ }, //    133
		{ 4, "\x48\x8d\x04\x07" /* l\xea   (%rdi,%rax,1),%rax*/ }, //    134
		{ 3, "\x40\x38\x30" /* cmp    %sil,(%rax)*/ }, //    135
		{ 4, "\x48\x0f\x45\xc2" /* cmovne %rdx,%rax*/ }, //    136
		{ 1, "\xc3" /* retq   */ }, //    137
		{ 1, "\x90" /* nop*/ }, //    138
		{ 5, "\xf3\x0f\x6f\x47\x10" /* movdqu 0x\x10(%rdi),%xmm0*/ }, //    139
		{ 4, "\x66\x0f\x6f\xe0" /* movdqa %xmm0,%xmm4*/ }, //    140
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    141
		{ 4, "\x66\x0f\x74\xe3" /* pcmpeqb %xmm3,%xmm4*/ }, //    142
		{ 4, "\x66\x0f\xeb\xc4" /* por    %xmm4,%xmm0*/ }, //    143
		{ 4, "\x66\x0f\xd7\xc8" /* pmovmskb %xmm0,%\xecx*/ }, //    144
		{ 5, "\xf3\x0f\x6f\x47\x20" /* movdqu 0x\x20(%rdi),%xmm0*/ }, //    145
		{ 4, "\x66\x0f\x6f\xe0" /* movdqa %xmm0,%xmm4*/ }, //    146
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    147
		{ 4, "\x48\xc1\xe1\x10" /* shl    $0x\x10,%rcx*/ }, //    148
		{ 4, "\x66\x0f\x74\xe3" /* pcmpeqb %xmm3,%xmm4*/ }, //    149
		{ 4, "\x66\x0f\xeb\xc4" /* por    %xmm4,%xmm0*/ }, //    150
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    151
		{ 5, "\xf3\x0f\x6f\x47\x30" /* movdqu 0x\x30(%rdi),%xmm0*/ }, //    152
		{ 4, "\x66\x0f\x74\xd8" /* pcmpeqb %xmm0,%xmm3*/ }, //    153
		{ 4, "\x48\xc1\xe0\x20" /* shl    $0x\x20,%rax*/ }, //    154
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    155
		{ 3, "\x48\x09\xc8" /* or     %rcx,%rax*/ }, //    156
		{ 4, "\x66\x0f\xeb\xc3" /* por    %xmm3,%xmm0*/ }, //    157
		{ 4, "\x66\x0f\xd7\xc8" /* pmovmskb %xmm0,%\xecx*/ }, //    158
		{ 4, "\x48\xc1\xe1\x30" /* shl    $0x\x30,%rcx*/ }, //    159
		{ 3, "\x48\x09\xc8" /* or     %rcx,%rax*/ }, //    160
		{ 3, "\x48\x85\xc0" /* test   %rax,%rax*/ }, //    161
		{ 6, "\x0f\x85\xa6\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x4b\xb0*/ }, //    162
		{ 6, "\x66\x0f\x1f\x44\x00\x00" /* nopw   0x0(%rax,%rax,1)*/ }, //    163
		{ 4, "\x66\x0f\xef\xf6" /* pxor   %xmm6,%xmm6*/ }, //    164
		{ 4, "\x48\x83\xe7\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rdi*/ }, //    165
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    166
		{ 4, "\x66\x0f\x6f\x2f" /* movdqa (%rdi),%xmm5*/ }, //    167
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    168
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    169
		{ 4, "\x66\x0f\xef\xe9" /* pxor   %xmm1,%xmm5*/ }, //    170
		{ 5, "\x66\x0f\x6f\x67\x30" /* movdqa 0x\x30(%rdi),%xmm4*/ }, //    171
		{ 4, "\x66\x0f\xef\xd1" /* pxor   %xmm1,%xmm2*/ }, //    172
		{ 4, "\x66\x0f\xef\xd9" /* pxor   %xmm1,%xmm3*/ }, //    173
		{ 4, "\x66\x0f\xda\x2f" /* pminub (%rdi),%xmm5*/ }, //    174
		{ 4, "\x66\x0f\xef\xe1" /* pxor   %xmm1,%xmm4*/ }, //    175
		{ 5, "\x66\x0f\xda\x57\x10" /* pminub 0x\x10(%rdi),%xmm2*/ }, //    176
		{ 5, "\x66\x0f\xda\x5f\x20" /* pminub 0x\x20(%rdi),%xmm3*/ }, //    177
		{ 4, "\x66\x0f\xda\xea" /* pminub %xmm2,%xmm5*/ }, //    178
		{ 5, "\x66\x0f\xda\x67\x30" /* pminub 0x\x30(%rdi),%xmm4*/ }, //    179
		{ 4, "\x66\x0f\xda\xeb" /* pminub %xmm3,%xmm5*/ }, //    180
		{ 4, "\x66\x0f\xda\xec" /* pminub %xmm4,%xmm5*/ }, //    181
		{ 4, "\x66\x0f\x74\xee" /* pcmpeqb %xmm6,%xmm5*/ }, //    182
		{ 4, "\x66\x0f\xd7\xc5" /* pmovmskb %xmm5,%\xeax*/ }, //    183
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    184
		{ 2, "\x74\xae" /* je     0x\x2a\xaa\xaa\xac\x4b\x18*/ }, //    185
		{ 4, "\x66\x0f\x6f\x2f" /* movdqa (%rdi),%xmm5*/ }, //    186
		{ 4, "\x66\x0f\x6f\xc5" /* movdqa %xmm5,%xmm0*/ }, //    187
		{ 4, "\x66\x0f\x74\xe9" /* pcmpeqb %xmm1,%xmm5*/ }, //    188
		{ 4, "\x66\x0f\x74\xc6" /* pcmpeqb %xmm6,%xmm0*/ }, //    189
		{ 4, "\x66\x0f\xeb\xe8" /* por    %xmm0,%xmm5*/ }, //    190
		{ 4, "\x66\x0f\x74\xd6" /* pcmpeqb %xmm6,%xmm2*/ }, //    191
		{ 4, "\x66\x0f\x74\xde" /* pcmpeqb %xmm6,%xmm3*/ }, //    192
		{ 4, "\x66\x0f\x74\xe6" /* pcmpeqb %xmm6,%xmm4*/ }, //    193
		{ 4, "\x66\x0f\xd7\xcd" /* pmovmskb %xmm5,%\xecx*/ }, //    194
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    195
		{ 4, "\x48\xc1\xe0\x10" /* shl    $0x\x10,%rax*/ }, //    196
		{ 5, "\x66\x44\x0f\xd7\xc3" /* pmovmskb %xmm3,%r\x8d*/ }, //    197
		{ 4, "\x66\x0f\xd7\xd4" /* pmovmskb %xmm4,%\xedx*/ }, //    198
		{ 4, "\x49\xc1\xe0\x20" /* shl    $0x\x20,%r8*/ }, //    199
		{ 3, "\x4c\x09\xc0" /* or     %r8,%rax*/ }, //    200
		{ 3, "\x48\x09\xc8" /* or     %rcx,%rax*/ }, //    201
		{ 4, "\x48\xc1\xe2\x30" /* shl    $0x\x30,%rdx*/ }, //    202
		{ 3, "\x48\x09\xd0" /* or     %rdx,%rax*/ }, //    203
		{ 4, "\x48\x0f\xbc\xc0" /* bsf    %rax,%rax*/ }, //    204
		{ 5, "\xba\x00\x00\x00\x00" /* mov    $0x0,%\xedx*/ }, //    205
		{ 4, "\x48\x8d\x04\x07" /* l\xea   (%rdi,%rax,1),%rax*/ }, //    206
		{ 3, "\x40\x38\x30" /* cmp    %sil,(%rax)*/ }, //    207
		{ 4, "\x48\x0f\x45\xc2" /* cmovne %rdx,%rax*/ }, //    208
		{ 1, "\xc3" /* retq   */ }, //    209
		{ 11, "\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00" /* \xdat\xa16 nopw %cs:0x0(%rax,%rax,1)*/ }, //    210
		{ 3, "\x48\x89\xfa" /* mov    %rdi,%rdx*/ }, //    211
		{ 4, "\x66\x0f\xef\xd2" /* pxor   %xmm2,%xmm2*/ }, //    212
		{ 4, "\x48\x83\xe2\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rdx*/ }, //    213
		{ 4, "\x66\x0f\x6f\xc1" /* movdqa %xmm1,%xmm0*/ }, //    214
		{ 4, "\x66\x0f\x6f\x1a" /* movdqa (%rdx),%xmm3*/ }, //    215
		{ 4, "\x66\x0f\x6f\xe3" /* movdqa %xmm3,%xmm4*/ }, //    216
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    217
		{ 4, "\x66\x0f\x74\xe2" /* pcmpeqb %xmm2,%xmm4*/ }, //    218
		{ 4, "\x66\x0f\xeb\xdc" /* por    %xmm4,%xmm3*/ }, //    219
		{ 5, "\x66\x44\x0f\xd7\xc3" /* pmovmskb %xmm3,%r\x8d*/ }, //    220
		{ 5, "\x66\x0f\x6f\x5a\x10" /* movdqa 0x\x10(%rdx),%xmm3*/ }, //    221
		{ 4, "\x66\x0f\x6f\xe3" /* movdqa %xmm3,%xmm4*/ }, //    222
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    223
		{ 4, "\x66\x0f\x74\xe2" /* pcmpeqb %xmm2,%xmm4*/ }, //    224
		{ 4, "\x66\x0f\xeb\xdc" /* por    %xmm4,%xmm3*/ }, //    225
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    226
		{ 5, "\x66\x0f\x6f\x5a\x20" /* movdqa 0x\x20(%rdx),%xmm3*/ }, //    227
		{ 4, "\x66\x0f\x6f\xe3" /* movdqa %xmm3,%xmm4*/ }, //    228
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    229
		{ 4, "\x48\xc1\xe0\x10" /* shl    $0x\x10,%rax*/ }, //    230
		{ 4, "\x66\x0f\x74\xe2" /* pcmpeqb %xmm2,%xmm4*/ }, //    231
		{ 4, "\x66\x0f\xeb\xdc" /* por    %xmm4,%xmm3*/ }, //    232
		{ 5, "\x66\x44\x0f\xd7\xcb" /* pmovmskb %xmm3,%r\x9d*/ }, //    233
		{ 5, "\x66\x0f\x6f\x5a\x30" /* movdqa 0x\x30(%rdx),%xmm3*/ }, //    234
		{ 4, "\x66\x0f\x74\xd3" /* pcmpeqb %xmm3,%xmm2*/ }, //    235
		{ 4, "\x49\xc1\xe1\x20" /* shl    $0x\x20,%r9*/ }, //    236
		{ 4, "\x66\x0f\x74\xc3" /* pcmpeqb %xmm3,%xmm0*/ }, //    237
		{ 3, "\x4c\x09\xc8" /* or     %r9,%rax*/ }, //    238
		{ 3, "\x4c\x09\xc0" /* or     %r8,%rax*/ }, //    239
		{ 4, "\x66\x0f\xeb\xc2" /* por    %xmm2,%xmm0*/ }, //    240
		{ 4, "\x66\x0f\xd7\xc8" /* pmovmskb %xmm0,%\xecx*/ }, //    241
		{ 4, "\x48\xc1\xe1\x30" /* shl    $0x\x30,%rcx*/ }, //    242
		{ 3, "\x48\x09\xc8" /* or     %rcx,%rax*/ }, //    243
		{ 2, "\x89\xf9" /* mov    %\xedi,%\xecx*/ }, //    244
		{ 2, "\x28\xd1" /* sub    %dl,%cl*/ }, //    245
		{ 3, "\x48\xd3\xe8" /* shr    %cl,%rax*/ }, //    246
		{ 3, "\x48\x85\xc0" /* test   %rax,%rax*/ }, //    247
		{ 6, "\x0f\x85\x4b\xff\xff\xff" /* jne    0x\x2a\xaa\xaa\xac\x4b\xb0*/ }, //    248
		{ 5, "\xe9\xa0\xfe\xff\xff" /* jmpq   0x\x2a\xaa\xaa\xac\x4b\x0a*/ }, //    249
		{ 6, "\x66\x0f\x1f\x44\x00\x00" /* nopw   0x0(%rax,%rax,1)*/ }, //    250
		{ 2, "\x8a\x07" /* mov    (%rdi),%al*/ }, //    251
		{ 2, "\x3a\x06" /* cmp    (%rsi),%al*/ }, //    252
		{ 2, "\x75\x0d" /* jne    0x\x2a\xaa\xaa\xac\x4c\x83*/ }, //    253
		{ 3, "\x48\xff\xc7" /* inc    %rdi*/ }, //    254
		{ 3, "\x48\xff\xc6" /* inc    %rsi*/ }, //    255
		{ 2, "\x84\xc0" /* test   %al,%al*/ }, //    256
		{ 2, "\x75\xf0" /* jne    0x\x2a\xaa\xaa\xac\x4c\x70*/ }, //    257
		{ 2, "\x31\xc0" /* xor    %\xeax,%\xeax*/ }, //    258
		{ 1, "\xc3" /* retq   */ }, //    259
		{ 5, "\xb8\x01\x00\x00\x00" /* mov    $0x1,%\xeax*/ }, //    260
		{ 5, "\xb9\xff\xff\xff\xff" /* mov    $0x\xff\xff\xff\xff,%\xecx*/ }, //    261
		{ 3, "\x0f\x42\xc1" /* cmovb  %\xecx,%\xeax*/ }, //    262
		{ 1, "\xc3" /* retq   */ }, //    263
		{ 10, "\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00" /* nopw   %cs:0x0(%rax,%rax,1)*/ }, //    264
		{ 5, "\x0f\x1f\x44\x00\x00" /* nopl   0x0(%rax,%rax,1)*/ }, //    265
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    266
		{ 3, "\x83\xe1\x07" /* and    $0x7,%\xecx*/ }, //    267
		{ 3, "\x48\x89\xf8" /* mov    %rdi,%rax*/ }, //    268
		{ 2, "\x74\x15" /* je     0x\x2a\xaa\xaa\xac\x4c\xc0*/ }, //    269
		{ 2, "\xf7\xd9" /* neg    %\xecx*/ }, //    270
		{ 3, "\x83\xc1\x08" /* \xadd    $0x8,%\xecx*/ }, //    271
		{ 3, "\x80\x38\x00" /* cmpb   $0x0,(%rax)*/ }, //    272
		{ 6, "\x0f\x84\xcc\x00\x00\x00" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    273
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    274
		{ 2, "\xff\xc9" /* \xdec    %\xecx*/ }, //    275
		{ 2, "\x75\xf0" /* jne    0x\x2a\xaa\xaa\xac\x4c\xb0*/ }, //    276
		{ 10, "\x49\xb8\xff\xfe\xfe\xfe\xfe\xfe\xfe\xfe" /* mov\xabs $0x\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xff,%r8*/ }, //    277
		{ 6, "\x66\x0f\x1f\x44\x00\x00" /* nopw   0x0(%rax,%rax,1)*/ }, //    278
		{ 3, "\x48\x8b\x08" /* mov    (%rax),%rcx*/ }, //    279
		{ 4, "\x48\x83\xc0\x08" /* \xadd    $0x8,%rax*/ }, //    280
		{ 3, "\x4c\x89\xc2" /* mov    %r8,%rdx*/ }, //    281
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    282
		{ 2, "\x73\x61" /* j\xae   0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    283
		{ 3, "\x48\x31\xca" /* xor    %rcx,%rdx*/ }, //    284
		{ 3, "\x4c\x09\xc2" /* or     %r8,%rdx*/ }, //    285
		{ 3, "\x48\xff\xc2" /* inc    %rdx*/ }, //    286
		{ 2, "\x75\x56" /* jne    0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    287
		{ 3, "\x48\x8b\x08" /* mov    (%rax),%rcx*/ }, //    288
		{ 4, "\x48\x83\xc0\x08" /* \xadd    $0x8,%rax*/ }, //    289
		{ 3, "\x4c\x89\xc2" /* mov    %r8,%rdx*/ }, //    290
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    291
		{ 2, "\x73\x47" /* j\xae   0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    292
		{ 3, "\x48\x31\xca" /* xor    %rcx,%rdx*/ }, //    293
		{ 3, "\x4c\x09\xc2" /* or     %r8,%rdx*/ }, //    294
		{ 3, "\x48\xff\xc2" /* inc    %rdx*/ }, //    295
		{ 2, "\x75\x3c" /* jne    0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    296
		{ 3, "\x48\x8b\x08" /* mov    (%rax),%rcx*/ }, //    297
		{ 4, "\x48\x83\xc0\x08" /* \xadd    $0x8,%rax*/ }, //    298
		{ 3, "\x4c\x89\xc2" /* mov    %r8,%rdx*/ }, //    299
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    300
		{ 2, "\x73\x2d" /* j\xae   0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    301
		{ 3, "\x48\x31\xca" /* xor    %rcx,%rdx*/ }, //    302
		{ 3, "\x4c\x09\xc2" /* or     %r8,%rdx*/ }, //    303
		{ 3, "\x48\xff\xc2" /* inc    %rdx*/ }, //    304
		{ 2, "\x75\x22" /* jne    0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    305
		{ 3, "\x48\x8b\x08" /* mov    (%rax),%rcx*/ }, //    306
		{ 4, "\x48\x83\xc0\x08" /* \xadd    $0x8,%rax*/ }, //    307
		{ 3, "\x4c\x89\xc2" /* mov    %r8,%rdx*/ }, //    308
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    309
		{ 2, "\x73\x13" /* j\xae   0x\x2a\xaa\xaa\xac\x4d\x40*/ }, //    310
		{ 3, "\x48\x31\xca" /* xor    %rcx,%rdx*/ }, //    311
		{ 3, "\x4c\x09\xc2" /* or     %r8,%rdx*/ }, //    312
		{ 3, "\x48\xff\xc2" /* inc    %rdx*/ }, //    313
		{ 2, "\x74\x98" /* je     0x\x2a\xaa\xaa\xac\x4c\xd0*/ }, //    314
		{ 8, "\x0f\x1f\x84\x00\x00\x00\x00\x00" /* nopl   0x0(%rax,%rax,1)*/ }, //    315
		{ 4, "\x48\x83\xe8\x08" /* sub    $0x8,%rax*/ }, //    316
		{ 2, "\x84\xc9" /* test   %cl,%cl*/ }, //    317
		{ 2, "\x74\x3d" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    318
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    319
		{ 2, "\x84\xed" /* test   %ch,%ch*/ }, //    320
		{ 2, "\x74\x36" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    321
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    322
		{ 6, "\xf7\xc1\x00\x00\xff\x00" /* test   $0x\xff\x00\x00,%\xecx*/ }, //    323
		{ 2, "\x74\x2b" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    324
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    325
		{ 6, "\xf7\xc1\x00\x00\x00\xff" /* test   $0x\xff\x00\x00\x00,%\xecx*/ }, //    326
		{ 2, "\x74\x20" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    327
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    328
		{ 4, "\x48\xc1\xe9\x20" /* shr    $0x\x20,%rcx*/ }, //    329
		{ 2, "\x84\xc9" /* test   %cl,%cl*/ }, //    330
		{ 2, "\x74\x15" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    331
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    332
		{ 2, "\x84\xed" /* test   %ch,%ch*/ }, //    333
		{ 2, "\x74\x0e" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    334
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    335
		{ 6, "\xf7\xc1\x00\x00\xff\x00" /* test   $0x\xff\x00\x00,%\xecx*/ }, //    336
		{ 2, "\x74\x03" /* je     0x\x2a\xaa\xaa\xac\x4d\x85*/ }, //    337
		{ 3, "\x48\xff\xc0" /* inc    %rax*/ }, //    338
		{ 3, "\x48\x29\xf8" /* sub    %rdi,%rax*/ }, //    339
		{ 1, "\xc3" /* retq   */ }, //    340
		{ 7, "\x0f\x1f\x80\x00\x00\x00\x00" /* nopl   0x0(%rax)*/ }, //    341
		{ 3, "\x48\x85\xf6" /* test   %rsi,%rsi*/ }, //    342
		{ 2, "\x75\x04" /* jne    0x\x2a\xaa\xaa\xac\x4d\x99*/ }, //    343
		{ 3, "\x48\x31\xc0" /* xor    %rax,%rax*/ }, //    344
		{ 1, "\xc3" /* retq   */ }, //    345
		{ 3, "\x48\x01\xfe" /* \xadd    %rdi,%rsi*/ }, //    346
		{ 3, "\x49\x89\xf2" /* mov    %rsi,%r\x10*/ }, //    347
		{ 4, "\x49\x83\xe2\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%r\x10*/ }, //    348
		{ 3, "\x49\x89\xf3" /* mov    %rsi,%r\x11*/ }, //    349
		{ 5, "\x66\x45\x0f\xef\xc0" /* pxor   %xmm8,%xmm8*/ }, //    350
		{ 5, "\x66\x45\x0f\xef\xc9" /* pxor   %xmm9,%xmm9*/ }, //    351
		{ 5, "\x66\x45\x0f\xef\xd2" /* pxor   %xmm\x10,%xmm\x10*/ }, //    352
		{ 5, "\x66\x45\x0f\xef\xdb" /* pxor   %xmm\x11,%xmm\x11*/ }, //    353
		{ 3, "\x48\x89\xf8" /* mov    %rdi,%rax*/ }, //    354
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    355
		{ 7, "\x48\x81\xe1\xff\x0f\x00\x00" /* and    $0x\xfff,%rcx*/ }, //    356
		{ 7, "\x48\x81\xf9\xcf\x0f\x00\x00" /* cmp    $0x\xfcf,%rcx*/ }, //    357
		{ 2, "\x77\x70" /* ja     0x\x2a\xaa\xaa\xac\x4e\x40*/ }, //    358
		{ 4, "\x48\x83\xe0\xf0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xf0,%rax*/ }, //    359
		{ 5, "\x66\x44\x0f\x74\x00" /* pcmpeqb (%rax),%xmm8*/ }, //    360
		{ 6, "\x66\x44\x0f\x74\x48\x10" /* pcmpeqb 0x\x10(%rax),%xmm9*/ }, //    361
		{ 6, "\x66\x44\x0f\x74\x50\x20" /* pcmpeqb 0x\x20(%rax),%xmm\x10*/ }, //    362
		{ 6, "\x66\x44\x0f\x74\x58\x30" /* pcmpeqb 0x\x30(%rax),%xmm\x11*/ }, //    363
		{ 5, "\x66\x41\x0f\xd7\xf0" /* pmovmskb %xmm8,%esi*/ }, //    364
		{ 5, "\x66\x41\x0f\xd7\xd1" /* pmovmskb %xmm9,%\xedx*/ }, //    365
		{ 5, "\x66\x45\x0f\xd7\xc2" /* pmovmskb %xmm\x10,%r\x8d*/ }, //    366
		{ 5, "\x66\x41\x0f\xd7\xcb" /* pmovmskb %xmm\x11,%\xecx*/ }, //    367
		{ 4, "\x48\xc1\xe2\x10" /* shl    $0x\x10,%rdx*/ }, //    368
		{ 4, "\x48\xc1\xe1\x10" /* shl    $0x\x10,%rcx*/ }, //    369
		{ 3, "\x48\x09\xf2" /* or     %rsi,%rdx*/ }, //    370
		{ 3, "\x4c\x09\xc1" /* or     %r8,%rcx*/ }, //    371
		{ 4, "\x48\xc1\xe1\x20" /* shl    $0x\x20,%rcx*/ }, //    372
		{ 3, "\x48\x09\xca" /* or     %rcx,%rdx*/ }, //    373
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    374
		{ 3, "\x48\x31\xc1" /* xor    %rax,%rcx*/ }, //    375
		{ 3, "\x4c\x89\xde" /* mov    %r\x11,%rsi*/ }, //    376
		{ 3, "\x48\x29\xc6" /* sub    %rax,%rsi*/ }, //    377
		{ 4, "\x48\x83\xe0\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rax*/ }, //    378
		{ 7, "\x48\xf7\xc6\xc0\xff\xff\xff" /* test   $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rsi*/ }, //    379
		{ 2, "\x74\x7d" /* je     0x\x2a\xaa\xaa\xac\x4e\xaa*/ }, //    380
		{ 3, "\x48\xd3\xfa" /* sar    %cl,%rdx*/ }, //    381
		{ 3, "\x48\x85\xd2" /* test   %rdx,%rdx*/ }, //    382
		{ 6, "\x0f\x84\x97\x00\x00\x00" /* je     0x\x2a\xaa\xaa\xac\x4e\xd0*/ }, //    383
		{ 4, "\x48\x0f\xbc\xc2" /* bsf    %rdx,%rax*/ }, //    384
		{ 1, "\xc3" /* retq   */ }, //    385
		{ 2, "\x66\x90" /* xchg   %ax,%ax*/ }, //    386
		{ 4, "\x48\x83\xe0\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rax*/ }, //    387
		{ 5, "\x66\x44\x0f\x74\x00" /* pcmpeqb (%rax),%xmm8*/ }, //    388
		{ 6, "\x66\x44\x0f\x74\x48\x10" /* pcmpeqb 0x\x10(%rax),%xmm9*/ }, //    389
		{ 6, "\x66\x44\x0f\x74\x50\x20" /* pcmpeqb 0x\x20(%rax),%xmm\x10*/ }, //    390
		{ 6, "\x66\x44\x0f\x74\x58\x30" /* pcmpeqb 0x\x30(%rax),%xmm\x11*/ }, //    391
		{ 5, "\x66\x41\x0f\xd7\xf0" /* pmovmskb %xmm8,%esi*/ }, //    392
		{ 5, "\x66\x41\x0f\xd7\xd1" /* pmovmskb %xmm9,%\xedx*/ }, //    393
		{ 5, "\x66\x45\x0f\xd7\xc2" /* pmovmskb %xmm\x10,%r\x8d*/ }, //    394
		{ 5, "\x66\x41\x0f\xd7\xcb" /* pmovmskb %xmm\x11,%\xecx*/ }, //    395
		{ 4, "\x48\xc1\xe2\x10" /* shl    $0x\x10,%rdx*/ }, //    396
		{ 4, "\x48\xc1\xe1\x10" /* shl    $0x\x10,%rcx*/ }, //    397
		{ 3, "\x48\x09\xf2" /* or     %rsi,%rdx*/ }, //    398
		{ 3, "\x4c\x09\xc1" /* or     %r8,%rcx*/ }, //    399
		{ 4, "\x48\xc1\xe1\x20" /* shl    $0x\x20,%rcx*/ }, //    400
		{ 3, "\x48\x09\xca" /* or     %rcx,%rdx*/ }, //    401
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    402
		{ 3, "\x48\x31\xc1" /* xor    %rax,%rcx*/ }, //    403
		{ 3, "\x4c\x89\xde" /* mov    %r\x11,%rsi*/ }, //    404
		{ 3, "\x48\x29\xc6" /* sub    %rax,%rsi*/ }, //    405
		{ 4, "\x48\x83\xe0\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rax*/ }, //    406
		{ 7, "\x48\xf7\xc6\xc0\xff\xff\xff" /* test   $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rsi*/ }, //    407
		{ 2, "\x74\x0d" /* je     0x\x2a\xaa\xaa\xac\x4e\xaa*/ }, //    408
		{ 3, "\x48\xd3\xfa" /* sar    %cl,%rdx*/ }, //    409
		{ 3, "\x48\x85\xd2" /* test   %rdx,%rdx*/ }, //    410
		{ 2, "\x74\x1b" /* je     0x\x2a\xaa\xaa\xac\x4e\xc0*/ }, //    411
		{ 4, "\x48\x0f\xbc\xc2" /* bsf    %rdx,%rax*/ }, //    412
		{ 1, "\xc3" /* retq   */ }, //    413
		{ 4, "\x48\x0f\xab\xf2" /* bts    %rsi,%rdx*/ }, //    414
		{ 3, "\x48\xd3\xfa" /* sar    %cl,%rdx*/ }, //    415
		{ 3, "\x48\x85\xd2" /* test   %rdx,%rdx*/ }, //    416
		{ 2, "\x74\x0a" /* je     0x\x2a\xaa\xaa\xac\x4e\xc0*/ }, //    417
		{ 4, "\x48\x0f\xbc\xc2" /* bsf    %rdx,%rax*/ }, //    418
		{ 1, "\xc3" /* retq   */ }, //    419
		{ 5, "\x0f\x1f\x44\x00\x00" /* nopl   0x0(%rax,%rax,1)*/ }, //    420
		{ 5, "\x66\x45\x0f\xef\xc9" /* pxor   %xmm9,%xmm9*/ }, //    421
		{ 5, "\x66\x45\x0f\xef\xd2" /* pxor   %xmm\x10,%xmm\x10*/ }, //    422
		{ 5, "\x66\x45\x0f\xef\xdb" /* pxor   %xmm\x11,%xmm\x11*/ }, //    423
		{ 1, "\x90" /* nop*/ }, //    424
		{ 4, "\x48\x83\xc0\x40" /* \xadd    $0x\x40,%rax*/ }, //    425
		{ 3, "\x49\x39\xc2" /* cmp    %rax,%r\x10*/ }, //    426
		{ 2, "\x74\x27" /* je     0x\x2a\xaa\xaa\xac\x4f\x00*/ }, //    427
		{ 5, "\x66\x44\x0f\x6f\x00" /* movdqa (%rax),%xmm8*/ }, //    428
		{ 6, "\x66\x44\x0f\xda\x40\x10" /* pminub 0x\x10(%rax),%xmm8*/ }, //    429
		{ 6, "\x66\x44\x0f\xda\x40\x20" /* pminub 0x\x20(%rax),%xmm8*/ }, //    430
		{ 6, "\x66\x44\x0f\xda\x40\x30" /* pminub 0x\x30(%rax),%xmm8*/ }, //    431
		{ 5, "\x66\x45\x0f\x74\xc3" /* pcmpeqb %xmm\x11,%xmm8*/ }, //    432
		{ 5, "\x66\x41\x0f\xd7\xd0" /* pmovmskb %xmm8,%\xedx*/ }, //    433
		{ 2, "\x85\xd2" /* test   %\xedx,%\xedx*/ }, //    434
		{ 2, "\x75\x62" /* jne    0x\x2a\xaa\xaa\xac\x4f\x60*/ }, //    435
		{ 2, "\xeb\xd0" /* jmp    0x\x2a\xaa\xaa\xac\x4e\xd0*/ }, //    436
		{ 3, "\x49\x39\xc3" /* cmp    %rax,%r\x11*/ }, //    437
		{ 2, "\x74\x45" /* je     0x\x2a\xaa\xaa\xac\x4f\x4a*/ }, //    438
		{ 5, "\x66\x45\x0f\xef\xc0" /* pxor   %xmm8,%xmm8*/ }, //    439
		{ 5, "\x66\x44\x0f\x74\x00" /* pcmpeqb (%rax),%xmm8*/ }, //    440
		{ 6, "\x66\x44\x0f\x74\x48\x10" /* pcmpeqb 0x\x10(%rax),%xmm9*/ }, //    441
		{ 6, "\x66\x44\x0f\x74\x50\x20" /* pcmpeqb 0x\x20(%rax),%xmm\x10*/ }, //    442
		{ 6, "\x66\x44\x0f\x74\x58\x30" /* pcmpeqb 0x\x30(%rax),%xmm\x11*/ }, //    443
		{ 5, "\x66\x41\x0f\xd7\xf0" /* pmovmskb %xmm8,%esi*/ }, //    444
		{ 5, "\x66\x41\x0f\xd7\xd1" /* pmovmskb %xmm9,%\xedx*/ }, //    445
		{ 5, "\x66\x45\x0f\xd7\xc2" /* pmovmskb %xmm\x10,%r\x8d*/ }, //    446
		{ 5, "\x66\x41\x0f\xd7\xcb" /* pmovmskb %xmm\x11,%\xecx*/ }, //    447
		{ 4, "\x48\xc1\xe2\x10" /* shl    $0x\x10,%rdx*/ }, //    448
		{ 4, "\x48\xc1\xe1\x10" /* shl    $0x\x10,%rcx*/ }, //    449
		{ 3, "\x48\x09\xf2" /* or     %rsi,%rdx*/ }, //    450
		{ 3, "\x4c\x09\xc1" /* or     %r8,%rcx*/ }, //    451
		{ 4, "\x48\xc1\xe1\x20" /* shl    $0x\x20,%rcx*/ }, //    452
		{ 3, "\x48\x09\xca" /* or     %rcx,%rdx*/ }, //    453
		{ 4, "\x4c\x0f\xab\xda" /* bts    %r\x11,%rdx*/ }, //    454
		{ 4, "\x48\x0f\xbc\xd2" /* bsf    %rdx,%rdx*/ }, //    455
		{ 3, "\x48\x01\xd0" /* \xadd    %rdx,%rax*/ }, //    456
		{ 3, "\x48\x29\xf8" /* sub    %rdi,%rax*/ }, //    457
		{ 1, "\xc3" /* retq   */ }, //    458
		{ 7, "\x0f\x1f\x80\x00\x00\x00\x00" /* nopl   0x0(%rax)*/ }, //    459
		{ 5, "\x66\x45\x0f\xef\xc0" /* pxor   %xmm8,%xmm8*/ }, //    460
		{ 5, "\x66\x44\x0f\x74\x00" /* pcmpeqb (%rax),%xmm8*/ }, //    461
		{ 6, "\x66\x44\x0f\x74\x48\x10" /* pcmpeqb 0x\x10(%rax),%xmm9*/ }, //    462
		{ 6, "\x66\x44\x0f\x74\x50\x20" /* pcmpeqb 0x\x20(%rax),%xmm\x10*/ }, //    463
		{ 6, "\x66\x44\x0f\x74\x58\x30" /* pcmpeqb 0x\x30(%rax),%xmm\x11*/ }, //    464
		{ 5, "\x66\x41\x0f\xd7\xf0" /* pmovmskb %xmm8,%esi*/ }, //    465
		{ 5, "\x66\x41\x0f\xd7\xd1" /* pmovmskb %xmm9,%\xedx*/ }, //    466
		{ 5, "\x66\x45\x0f\xd7\xc2" /* pmovmskb %xmm\x10,%r\x8d*/ }, //    467
		{ 5, "\x66\x41\x0f\xd7\xcb" /* pmovmskb %xmm\x11,%\xecx*/ }, //    468
		{ 4, "\x48\xc1\xe2\x10" /* shl    $0x\x10,%rdx*/ }, //    469
		{ 4, "\x48\xc1\xe1\x10" /* shl    $0x\x10,%rcx*/ }, //    470
		{ 3, "\x48\x09\xf2" /* or     %rsi,%rdx*/ }, //    471
		{ 3, "\x4c\x09\xc1" /* or     %r8,%rcx*/ }, //    472
		{ 4, "\x48\xc1\xe1\x20" /* shl    $0x\x20,%rcx*/ }, //    473
		{ 3, "\x48\x09\xca" /* or     %rcx,%rdx*/ }, //    474
		{ 4, "\x48\x0f\xbc\xd2" /* bsf    %rdx,%rdx*/ }, //    475
		{ 3, "\x48\x01\xd0" /* \xadd    %rdx,%rax*/ }, //    476
		{ 3, "\x48\x29\xf8" /* sub    %rdi,%rax*/ }, //    477
		{ 1, "\xc3" /* retq   */ }, //    478
		{ 5, "\x66\x48\x0f\x6e\xce" /* movq   %rsi,%xmm1*/ }, //    479
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    480
		{ 4, "\x66\x0f\x60\xc9" /* punpcklbw %xmm1,%xmm1*/ }, //    481
		{ 3, "\x48\x85\xd2" /* test   %rdx,%rdx*/ }, //    482
		{ 6, "\x0f\x84\x2b\x03\x00\x00" /* je     0x\x2a\xaa\xaa\xac\x52\xf0*/ }, //    483
		{ 4, "\x66\x0f\x60\xc9" /* punpcklbw %xmm1,%xmm1*/ }, //    484
		{ 4, "\x48\x83\xe1\x3f" /* and    $0x\x3f,%rcx*/ }, //    485
		{ 5, "\x66\x0f\x70\xc9\x00" /* pshu\xfd$0x0,%xmm1,%xmm1*/ }, //    486
		{ 4, "\x48\x83\xf9\x30" /* cmp    $0x\x30,%rcx*/ }, //    487
		{ 2, "\x77\x48" /* ja     0x\x2a\xaa\xaa\xac\x50\x20*/ }, //    488
		{ 4, "\xf3\x0f\x6f\x07" /* movdqu (%rdi),%xmm0*/ }, //    489
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    490
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    491
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    492
		{ 6, "\x0f\x85\xc4\x02\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\xb0*/ }, //    493
		{ 4, "\x48\x83\xea\x10" /* sub    $0x\x10,%rdx*/ }, //    494
		{ 6, "\x0f\x86\xfa\x02\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x52\xf0*/ }, //    495
		{ 4, "\x48\x83\xc7\x10" /* \xadd    $0x\x10,%rdi*/ }, //    496
		{ 4, "\x48\x83\xe1\x0f" /* and    $0xf,%rcx*/ }, //    497
		{ 4, "\x48\x83\xe7\xf0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xf0,%rdi*/ }, //    498
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    499
		{ 4, "\x48\x83\xea\x40" /* sub    $0x\x40,%rdx*/ }, //    500
		{ 6, "\x0f\x86\xc1\x01\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x51\xd0*/ }, //    501
		{ 2, "\xeb\x5f" /* jmp    0x\x2a\xaa\xaa\xac\x50\x70*/ }, //    502
		{ 15, "\x66\x66\x66\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00" /* \xdat\xa16 \xdat\xa16 \xdat\xa16 \xdat\xa1*/ }, //    503
		{ 4, "\x48\x83\xe1\x0f" /* and    $0xf,%rcx*/ }, //    504
		{ 4, "\x48\x83\xe7\xf0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xf0,%rdi*/ }, //    505
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    506
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    507
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    508
		{ 2, "\xd3\xf8" /* sar    %cl,%\xeax*/ }, //    509
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    510
		{ 2, "\x74\x16" /* je     0x\x2a\xaa\xaa\xac\x50\x50*/ }, //    511
		{ 3, "\x0f\xbc\xc0" /* bsf    %\xeax,%\xeax*/ }, //    512
		{ 3, "\x48\x29\xc2" /* sub    %rax,%rdx*/ }, //    513
		{ 6, "\x0f\x86\xaa\x02\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x52\xf0*/ }, //    514
		{ 3, "\x48\x01\xf8" /* \xadd    %rdi,%rax*/ }, //    515
		{ 3, "\x48\x01\xc8" /* \xadd    %rcx,%rax*/ }, //    516
		{ 1, "\xc3" /* retq   */ }, //    517
		{ 3, "\x0f\x1f\x00" /* nopl   (%rax)*/ }, //    518
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    519
		{ 4, "\x48\x83\xea\x10" /* sub    $0x\x10,%rdx*/ }, //    520
		{ 6, "\x0f\x86\x93\x02\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x52\xf0*/ }, //    521
		{ 4, "\x48\x83\xc7\x10" /* \xadd    $0x\x10,%rdi*/ }, //    522
		{ 4, "\x48\x83\xea\x40" /* sub    $0x\x40,%rdx*/ }, //    523
		{ 6, "\x0f\x86\x65\x01\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x51\xd0*/ }, //    524
		{ 5, "\x0f\x1f\x44\x00\x00" /* nopl   0x0(%rax,%rax,1)*/ }, //    525
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    526
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    527
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    528
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    529
		{ 6, "\x0f\x85\xfc\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x80*/ }, //    530
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    531
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    532
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    533
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    534
		{ 6, "\x0f\x85\xf7\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x90*/ }, //    535
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    536
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    537
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    538
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    539
		{ 6, "\x0f\x85\xf2\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\xa0*/ }, //    540
		{ 5, "\x66\x0f\x6f\x67\x30" /* movdqa 0x\x30(%rdi),%xmm4*/ }, //    541
		{ 4, "\x66\x0f\x74\xe1" /* pcmpeqb %xmm1,%xmm4*/ }, //    542
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    543
		{ 4, "\x66\x0f\xd7\xc4" /* pmovmskb %xmm4,%\xeax*/ }, //    544
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    545
		{ 6, "\x0f\x85\xa9\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x70*/ }, //    546
		{ 7, "\x48\xf7\xc7\x3f\x00\x00\x00" /* test   $0x\x3f,%rdi*/ }, //    547
		{ 2, "\x74\x70" /* je     0x\x2a\xaa\xaa\xac\x51\x40*/ }, //    548
		{ 4, "\x48\x83\xea\x40" /* sub    $0x\x40,%rdx*/ }, //    549
		{ 6, "\x0f\x86\xf6\x00\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x51\xd0*/ }, //    550
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    551
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    552
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    553
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    554
		{ 6, "\x0f\x85\x92\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x80*/ }, //    555
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    556
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    557
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    558
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    559
		{ 6, "\x0f\x85\x8d\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x90*/ }, //    560
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    561
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    562
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    563
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    564
		{ 6, "\x0f\x85\x88\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\xa0*/ }, //    565
		{ 5, "\x66\x0f\x6f\x5f\x30" /* movdqa 0x\x30(%rdi),%xmm3*/ }, //    566
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    567
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    568
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    569
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    570
		{ 6, "\x0f\x85\x3f\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x70*/ }, //    571
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    572
		{ 4, "\x48\x83\xe7\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rdi*/ }, //    573
		{ 4, "\x48\x83\xe1\x3f" /* and    $0x\x3f,%rcx*/ }, //    574
		{ 3, "\x48\x01\xca" /* \xadd    %rcx,%rdx*/ }, //    575
		{ 1, "\x90" /* nop*/ }, //    576
		{ 4, "\x48\x83\xea\x40" /* sub    $0x\x40,%rdx*/ }, //    577
		{ 6, "\x0f\x86\x86\x00\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x51\xd0*/ }, //    578
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    579
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    580
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    581
		{ 5, "\x66\x0f\x6f\x67\x30" /* movdqa 0x\x30(%rdi),%xmm4*/ }, //    582
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    583
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    584
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    585
		{ 4, "\x66\x0f\x74\xe1" /* pcmpeqb %xmm1,%xmm4*/ }, //    586
		{ 4, "\x66\x0f\xde\xd8" /* pmaxub %xmm0,%xmm3*/ }, //    587
		{ 4, "\x66\x0f\xde\xe2" /* pmaxub %xmm2,%xmm4*/ }, //    588
		{ 4, "\x66\x0f\xde\xe3" /* pmaxub %xmm3,%xmm4*/ }, //    589
		{ 4, "\x66\x0f\xd7\xc4" /* pmovmskb %xmm4,%\xeax*/ }, //    590
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    591
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    592
		{ 2, "\x74\xbb" /* je     0x\x2a\xaa\xaa\xac\x51\x40*/ }, //    593
		{ 4, "\x48\x83\xef\x40" /* sub    $0x\x40,%rdi*/ }, //    594
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    595
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    596
		{ 6, "\x0f\x85\xeb\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x80*/ }, //    597
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    598
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    599
		{ 6, "\x0f\x85\xef\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x90*/ }, //    600
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    601
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    602
		{ 5, "\x66\x0f\x74\x4f\x30" /* pcmpeqb 0x\x30(%rdi),%xmm1*/ }, //    603
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    604
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    605
		{ 6, "\x0f\x85\xe5\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\xa0*/ }, //    606
		{ 4, "\x66\x0f\xd7\xc1" /* pmovmskb %xmm1,%\xeax*/ }, //    607
		{ 3, "\x0f\xbc\xc0" /* bsf    %\xeax,%\xeax*/ }, //    608
		{ 5, "\x48\x8d\x44\x07\x30" /* l\xea   0x\x30(%rdi,%rax,1),%rax*/ }, //    609
		{ 1, "\xc3" /* retq   */ }, //    610
		{ 8, "\x0f\x1f\x84\x00\x00\x00\x00\x00" /* nopl   0x0(%rax,%rax,1)*/ }, //    611
		{ 4, "\x48\x83\xc2\x20" /* \xadd    $0x\x20,%rdx*/ }, //    612
		{ 2, "\x7e\x6a" /* jle    0x\x2a\xaa\xaa\xac\x52\x40*/ }, //    613
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    614
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    615
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    616
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    617
		{ 6, "\x0f\x85\x96\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x80*/ }, //    618
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    619
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    620
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    621
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    622
		{ 6, "\x0f\x85\x91\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\x90*/ }, //    623
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    624
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    625
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    626
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    627
		{ 6, "\x0f\x85\xbc\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\xd0*/ }, //    628
		{ 4, "\x48\x83\xea\x10" /* sub    $0x\x10,%rdx*/ }, //    629
		{ 6, "\x0f\x8e\xd2\x00\x00\x00" /* jle    0x\x2a\xaa\xaa\xac\x52\xf0*/ }, //    630
		{ 5, "\x66\x0f\x74\x4f\x30" /* pcmpeqb 0x\x30(%rdi),%xmm1*/ }, //    631
		{ 4, "\x66\x0f\xd7\xc1" /* pmovmskb %xmm1,%\xeax*/ }, //    632
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    633
		{ 6, "\x0f\x85\xb1\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x52\xe0*/ }, //    634
		{ 3, "\x48\x31\xc0" /* xor    %rax,%rax*/ }, //    635
		{ 1, "\xc3" /* retq   */ }, //    636
		{ 13, "\x66\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00" /* \xdat\xa16 \xdat\xa16 \xdat\xa16 nopw %cs:0x0(*/ }, //    637
		{ 4, "\x48\x83\xc2\x20" /* \xadd    $0x\x20,%rdx*/ }, //    638
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    639
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    640
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    641
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    642
		{ 2, "\x75\x5c" /* jne    0x\x2a\xaa\xaa\xac\x52\xb0*/ }, //    643
		{ 4, "\x48\x83\xea\x10" /* sub    $0x\x10,%rdx*/ }, //    644
		{ 6, "\x0f\x86\x92\x00\x00\x00" /* j\xbe   0x\x2a\xaa\xaa\xac\x52\xf0*/ }, //    645
		{ 5, "\x66\x0f\x74\x4f\x10" /* pcmpeqb 0x\x10(%rdi),%xmm1*/ }, //    646
		{ 5, "\x66\x48\x0f\x6e\xce" /* movq   %rsi,%xmm1*/ }, //    647
		{ 3, "\x48\x89\xf9" /* mov    %rdi,%rcx*/ }, //    648
		{ 4, "\x66\x0f\x60\xc9" /* punpcklbw %xmm1,%xmm1*/ }, //    649
		{ 4, "\x66\x0f\x60\xc9" /* punpcklbw %xmm1,%xmm1*/ }, //    650
		{ 4, "\x48\x83\xe1\x3f" /* and    $0x\x3f,%rcx*/ }, //    651
		{ 5, "\x66\x0f\x70\xc9\x00" /* pshu\xfd$0x0,%xmm1,%xmm1*/ }, //    652
		{ 4, "\x48\x83\xf9\x30" /* cmp    $0x\x30,%rcx*/ }, //    653
		{ 2, "\x77\x21" /* ja     0x\x2a\xaa\xaa\xac\x62\xf0*/ }, //    654
		{ 4, "\xf3\x0f\x6f\x07" /* movdqu (%rdi),%xmm0*/ }, //    655
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    656
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    657
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    658
		{ 6, "\x0f\x85\x9d\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x80*/ }, //    659
		{ 4, "\x48\x83\xc7\x10" /* \xadd    $0x\x10,%rdi*/ }, //    660
		{ 4, "\x48\x83\xe7\xf0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xf0,%rdi*/ }, //    661
		{ 2, "\xeb\x43" /* jmp    0x\x2a\xaa\xaa\xac\x63\x30*/ }, //    662
		{ 3, "\x0f\x1f\x00" /* nopl   (%rax)*/ }, //    663
		{ 4, "\x48\x83\xe1\x0f" /* and    $0xf,%rcx*/ }, //    664
		{ 4, "\x48\x83\xe7\xf0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xf0,%rdi*/ }, //    665
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    666
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    667
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    668
		{ 2, "\xd3\xf8" /* sar    %cl,%\xeax*/ }, //    669
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    670
		{ 2, "\x74\x16" /* je     0x\x2a\xaa\xaa\xac\x63\x20*/ }, //    671
		{ 3, "\x0f\xbc\xc0" /* bsf    %\xeax,%\xeax*/ }, //    672
		{ 3, "\x48\x01\xf8" /* \xadd    %rdi,%rax*/ }, //    673
		{ 3, "\x48\x01\xc8" /* \xadd    %rcx,%rax*/ }, //    674
		{ 1, "\xc3" /* retq   */ }, //    675
		{ 12, "\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00" /* \xdat\xa16 \xdat\xa16 nopw %cs:0x0(%rax,%rax,1*/ }, //    676
		{ 4, "\x48\x83\xc7\x10" /* \xadd    $0x\x10,%rdi*/ }, //    677
		{ 12, "\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00" /* \xdat\xa16 \xdat\xa16 nopw %cs:0x0(%rax,%rax,1*/ }, //    678
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    679
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    680
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    681
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    682
		{ 6, "\x0f\x85\x3c\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x80*/ }, //    683
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    684
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    685
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    686
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    687
		{ 6, "\x0f\x85\x37\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x90*/ }, //    688
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    689
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    690
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    691
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    692
		{ 6, "\x0f\x85\x32\x01\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\xa0*/ }, //    693
		{ 5, "\x66\x0f\x6f\x67\x30" /* movdqa 0x\x30(%rdi),%xmm4*/ }, //    694
		{ 4, "\x66\x0f\x74\xe1" /* pcmpeqb %xmm1,%xmm4*/ }, //    695
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    696
		{ 4, "\x66\x0f\xd7\xc4" /* pmovmskb %xmm4,%\xeax*/ }, //    697
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    698
		{ 6, "\x0f\x85\xe9\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x70*/ }, //    699
		{ 7, "\x48\xf7\xc7\x3f\x00\x00\x00" /* test   $0x\x3f,%rdi*/ }, //    700
		{ 2, "\x74\x60" /* je     0x\x2a\xaa\xaa\xac\x63\xf0*/ }, //    701
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    702
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    703
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    704
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    705
		{ 6, "\x0f\x85\xdc\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x80*/ }, //    706
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    707
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    708
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    709
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    710
		{ 6, "\x0f\x85\xd7\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x90*/ }, //    711
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    712
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    713
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    714
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    715
		{ 6, "\x0f\x85\xd2\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\xa0*/ }, //    716
		{ 5, "\x66\x0f\x6f\x5f\x30" /* movdqa 0x\x30(%rdi),%xmm3*/ }, //    717
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    718
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    719
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    720
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    721
		{ 6, "\x0f\x85\x89\x00\x00\x00" /* jne    0x\x2a\xaa\xaa\xac\x64\x70*/ }, //    722
		{ 4, "\x48\x83\xe7\xc0" /* and    $0x\xff\xff\xff\xff\xff\xff\xff\xc0,%rdi*/ }, //    723
		{ 5, "\x0f\x1f\x44\x00\x00" /* nopl   0x0(%rax,%rax,1)*/ }, //    724
		{ 4, "\x66\x0f\x6f\x07" /* movdqa (%rdi),%xmm0*/ }, //    725
		{ 5, "\x66\x0f\x6f\x57\x10" /* movdqa 0x\x10(%rdi),%xmm2*/ }, //    726
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    727
		{ 5, "\x66\x0f\x6f\x67\x30" /* movdqa 0x\x30(%rdi),%xmm4*/ }, //    728
		{ 4, "\x66\x0f\x74\xc1" /* pcmpeqb %xmm1,%xmm0*/ }, //    729
		{ 4, "\x66\x0f\x74\xd1" /* pcmpeqb %xmm1,%xmm2*/ }, //    730
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    731
		{ 4, "\x66\x0f\x74\xe1" /* pcmpeqb %xmm1,%xmm4*/ }, //    732
		{ 4, "\x66\x0f\xde\xd8" /* pmaxub %xmm0,%xmm3*/ }, //    733
		{ 4, "\x66\x0f\xde\xe2" /* pmaxub %xmm2,%xmm4*/ }, //    734
		{ 4, "\x66\x0f\xde\xe3" /* pmaxub %xmm3,%xmm4*/ }, //    735
		{ 4, "\x66\x0f\xd7\xc4" /* pmovmskb %xmm4,%\xeax*/ }, //    736
		{ 4, "\x48\x83\xc7\x40" /* \xadd    $0x\x40,%rdi*/ }, //    737
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    738
		{ 2, "\x74\xc5" /* je     0x\x2a\xaa\xaa\xac\x63\xf0*/ }, //    739
		{ 4, "\x48\x83\xef\x40" /* sub    $0x\x40,%rdi*/ }, //    740
		{ 4, "\x66\x0f\xd7\xc0" /* pmovmskb %xmm0,%\xeax*/ }, //    741
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    742
		{ 2, "\x75\x49" /* jne    0x\x2a\xaa\xaa\xac\x64\x80*/ }, //    743
		{ 4, "\x66\x0f\xd7\xc2" /* pmovmskb %xmm2,%\xeax*/ }, //    744
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    745
		{ 2, "\x75\x51" /* jne    0x\x2a\xaa\xaa\xac\x64\x90*/ }, //    746
		{ 5, "\x66\x0f\x6f\x5f\x20" /* movdqa 0x\x20(%rdi),%xmm3*/ }, //    747
		{ 4, "\x66\x0f\x74\xd9" /* pcmpeqb %xmm1,%xmm3*/ }, //    748
		{ 5, "\x66\x0f\x74\x4f\x30" /* pcmpeqb 0x\x30(%rdi),%xmm1*/ }, //    749
		{ 4, "\x66\x0f\xd7\xc3" /* pmovmskb %xmm3,%\xeax*/ }, //    750
		{ 2, "\x85\xc0" /* test   %\xeax,%\xeax*/ }, //    751
		{ 2, "\x75\x4b" /* jne    0x\x2a\xaa\xaa\xac\x64\xa0*/ }, //    752
		{ 4, "\x66\x0f\xd7\xc1" /* pmovmskb %xmm1,%\xeax*/ }, //    753
};

int main(void)
{
	ud_t ud_obj;
	ud_init(&ud_obj);
	ud_set_mode(&ud_obj, 64);
	
	for (struct instr_test *t = &tests[0]; 
			t < &tests[sizeof tests / sizeof (struct instr_test)];
			++t)
	{
		int num = 1 + (t - &tests[0]);
		int len = instr_len(t->instr_bytes);
		if (len < 0)
		{
			warnx("Decode test %d failed at %p", num, t->instr_bytes);
			any_failed = 1;
		}
		else if (len != t->len)
		{
			warnx("Decode test %d incorrect at %p (gave %d, should be %d)", 
				num, t->instr_bytes, len, t->len);
			any_failed = 1;
		}
		
		// try udis86
		ud_set_input_buffer(&ud_obj, (const uint8_t *) t->instr_bytes, 15 /* HACK */);
		int ud_ret = ud_decode(&ud_obj);
		if (!ud_ret)
		{
			warnx("Decode test %d: libudis86 failed at %p", num, t->instr_bytes);
		}
		else
		{
			unsigned ud_len = ud_insn_len(&ud_obj);
			if (ud_len != t->len)
			{
				warnx("Decode test %d: libudis86 incorrect at %p (gave %d, should be %d)", 
					num, t->instr_bytes, (int) ud_len, t->len);
			}
		}
	}
	
	return any_failed;
}
