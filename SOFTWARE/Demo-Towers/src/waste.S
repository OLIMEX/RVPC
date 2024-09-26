// File: waste.S
// Purpose: Waste time (loop)
// Copyright (c) 2024 by Curtis Whitley
//
// License: Apache
//

//-----------------------------------------------------------

	.section .text
	.align 4

	.global waste_time

waste_time:                     // parameter is passed in a0 register
	addi    a0,a0,-1            // decrease count
	bne     a0,x0,waste_time    // back if more delay
	jr      x1                  // return from subroutine
