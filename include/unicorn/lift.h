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

#include "tcg/tcg.h"

TCGContext* aarch64_generate_tcg_lift(void* start_addr, uint32_t number_insn);

void aarch64_generate_tcg_init();
void aarch64_generate_tcg_reinit();

void generate_tcg_optimize(TCGContext* s);
void generate_tcg_print(TCGContext* s);

