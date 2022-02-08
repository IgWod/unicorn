/*
 *  AArch64 to TCG lifting
 *
 * Copyright 2024 Igor Wodiany
 * Copyright 2024 The University of Manchester
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

extern struct DisasContext gen_tcg_dc;

void aarch64_generate_tcg_insn(struct DisasContext *s);
void aarch64_generate_tcg_insn_start(struct DisasContext *s);

void aarch64_generate_tcg_init_context();
void aarch64_generate_tcg_reinit_context();
void aarch64_generate_tcg_init_a53();

