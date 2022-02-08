/*
 *  AArch64 to TCG lifting
 *
 *  Copyright (c) 2013 Alexander Graf <agraf@suse.de>
 *  Copyright 2024 Igor Wodiany
 *  Copyright 2024 The University of Manchester
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

// Global states used to generate TCG
ARMISARegisters gen_tcg_isar;
DisasContext gen_tcg_dc;

// For comments see disas_a64_insn
void aarch64_generate_tcg_insn(DisasContext *s)
{
    uint32_t insn;

    s->pc_curr = s->base.pc_next;

    // Loaded instruction directly from the given address
    insn = *((uint32_t*)s->pc_curr);

#ifdef TARGET_WORDS_BIGENDIAN
    insn = bswap32(insn);
#endif
    s->insn = insn;
    s->base.pc_next += 4;

    s->fp_access_checked = false;

    if (dc_isar_feature(aa64_bti, s)) {
        fprintf(stderr, "WARNING: BTI feature is not supported!\n");
    }

    switch (extract32(insn, 25, 4)) {
    case 0x0: case 0x1: case 0x3:
        unallocated_encoding(s);
        break;
    case 0x2:
        if (!dc_isar_feature(aa64_sve, s) || !disas_sve(s, insn)) {
            unallocated_encoding(s);
        }
        break;
    case 0x8: case 0x9:
        disas_data_proc_imm(s, insn);
        break;
    case 0xa: case 0xb:
        disas_b_exc_sys(s, insn);
        break;
    case 0x4:
    case 0x6:
    case 0xc:
    case 0xe:
        disas_ldst(s, insn);
        break;
    case 0x5:
    case 0xd:
        disas_data_proc_reg(s, insn);
        break;
    case 0x7:
    case 0xf:
        disas_data_proc_simd_fp(s, insn);
        break;
    default:
        assert(FALSE);
        break;
    }

    free_tmp_a64(s);

    if (s->btype > 0 && s->base.is_jmp != DISAS_NORETURN) {
        reset_btype(s);
    }
}

void aarch64_generate_tcg_insn_start(DisasContext *s)
{
    TCGContext *tcg_ctx = s->uc->tcg_ctx;

    tcg_gen_insn_start(tcg_ctx, s->base.pc_next, 0, 0);
    s->insn_start = tcg_last_op(tcg_ctx);
}


void aarch64_generate_tcg_init_context() {
    tcg_context_init(gen_tcg_dc.uc->tcg_ctx);
    arm_translate_init(gen_tcg_dc.uc);

    QTAILQ_INIT(&gen_tcg_dc.uc->tcg_ctx->ops);
    QTAILQ_INIT(&gen_tcg_dc.uc->tcg_ctx->free_ops);
    QSIMPLEQ_INIT(&gen_tcg_dc.uc->tcg_ctx->labels);
}

void aarch64_generate_tcg_reinit_context() {
    QTAILQ_INIT(&gen_tcg_dc.uc->tcg_ctx->ops);
}

void aarch64_generate_tcg_init_a53() {
    gen_tcg_dc.base.tb = NULL;
    gen_tcg_dc.base.pc_next = 0x400620;
    gen_tcg_dc.base.is_jmp = DISAS_NEXT;
    gen_tcg_dc.base.singlestep_enabled = 0;

    // ISAR setup for A53 (taken from cpu64.c)
    gen_tcg_isar.mvfr0 = 0x10110222;
    gen_tcg_isar.mvfr1 = 0x12111111;
    gen_tcg_isar.mvfr2 = 0x00000043;
    gen_tcg_isar.id_dfr0 = 0x03010066;
    gen_tcg_isar.id_mmfr0 = 0x10101105;
    gen_tcg_isar.id_mmfr1 = 0x40000000;
    gen_tcg_isar.id_mmfr2 = 0x01260000;
    gen_tcg_isar.id_mmfr3 = 0x02102211;
    gen_tcg_isar.id_isar0 = 0x02101110;
    gen_tcg_isar.id_isar1 = 0x13112111;
    gen_tcg_isar.id_isar2 = 0x21232042;
    gen_tcg_isar.id_isar3 = 0x01112131;
    gen_tcg_isar.id_isar4 = 0x00011142;
    gen_tcg_isar.id_isar5 = 0x00011121;
    gen_tcg_isar.id_isar6 = 0;
    gen_tcg_isar.id_aa64pfr0 = 0x00002222;
    gen_tcg_isar.id_aa64dfr0 = 0x10305106;
    gen_tcg_isar.id_isar0 = 0x00011120;
    gen_tcg_isar.id_aa64mmfr0 = 0x00001122;
    gen_tcg_isar.dbgdidr = 0x3516d000;

    gen_tcg_dc.isar = &gen_tcg_isar;

    gen_tcg_dc.be_data = MO_LE; // Alternative MO_BE
    gen_tcg_dc.mmu_idx = 0;
    gen_tcg_dc.tbii = 0;
    gen_tcg_dc.current_el = 0;
    gen_tcg_dc.fp_excp_el = 0;
    gen_tcg_dc.sve_excp_el = 0;
    gen_tcg_dc.pauth_active = 0;
    gen_tcg_dc.btype = 0;
    gen_tcg_dc.unpriv = 0;
    gen_tcg_dc.cp_regs = NULL;

    // Features setup for A53 (taken from cpu64.c)
    gen_tcg_dc.features = 0;
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_V8);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_NEON);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_GENERIC_TIMER);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_AARCH64);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_CBAR_RO);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_EL2);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_EL3);
    gen_tcg_dc.features |= (1ULL << ARM_FEATURE_PMU);

    gen_tcg_dc.ss_active = 0;
    gen_tcg_dc.is_ldex = 0;

    init_tmp_a64_array(&gen_tcg_dc);

    gen_tcg_dc.uc = (struct uc_struct*) malloc(sizeof(struct uc_struct));
    gen_tcg_dc.uc->no_exit_request = 1;

    gen_tcg_dc.uc->tcg_ctx = (TCGContext*) malloc(sizeof(TCGContext));

    aarch64_generate_tcg_init_context();

    gen_tcg_dc.uc->tcg_ctx->uc = gen_tcg_dc.uc;
}

