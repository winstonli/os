#pragma once

// initialise handlers for software/externally-generated interrupts
// must be called after idt_init and before enabling interrupts
void irq_init();
