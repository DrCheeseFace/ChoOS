#include <kernel/idt.h>
#include <kernel/keyboard.h>
#include <kernel/misc.h>
#include <kernel/tty.h>
#include <kernel/utils.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

global_variable bool CTRL_pressed;
global_variable bool SHIFT_pressed;
global_variable bool ALT_pressed;

const uint32_t lowercase[128] = { KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_ESC,
				  '1',
				  '2',
				  '3',
				  '4',
				  '5',
				  '6',
				  '7',
				  '8',
				  '9',
				  '0',
				  '-',
				  '=',
				  '\b',
				  '\t',
				  'q',
				  'w',
				  'e',
				  'r',
				  't',
				  'y',
				  'u',
				  'i',
				  'o',
				  'p',
				  '[',
				  ']',
				  '\n',
				  KEYBOARD_KEY_CTRL,
				  'a',
				  's',
				  'd',
				  'f',
				  'g',
				  'h',
				  'j',
				  'k',
				  'l',
				  ';',
				  '\'',
				  '`',
				  KEYBOARD_KEY_LSHFT,
				  '\\',
				  'z',
				  'x',
				  'c',
				  'v',
				  'b',
				  'n',
				  'm',
				  ',',
				  '.',
				  '/',
				  KEYBOARD_KEY_RSHFT,
				  '*',
				  KEYBOARD_KEY_ALT,
				  ' ',
				  KEYBOARD_KEY_CAPS,
				  KEYBOARD_KEY_F1,
				  KEYBOARD_KEY_F2,
				  KEYBOARD_KEY_F3,
				  KEYBOARD_KEY_F4,
				  KEYBOARD_KEY_F5,
				  KEYBOARD_KEY_F6,
				  KEYBOARD_KEY_F7,
				  KEYBOARD_KEY_F8,
				  KEYBOARD_KEY_F9,
				  KEYBOARD_KEY_F10,
				  KEYBOARD_KEY_NUMLCK,
				  KEYBOARD_KEY_SCRLCK,
				  KEYBOARD_KEY_HOME,
				  KEYBOARD_KEY_UP,
				  KEYBOARD_KEY_PGUP,
				  '-',
				  KEYBOARD_KEY_LEFT,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_RIGHT,
				  '+',
				  KEYBOARD_KEY_END,
				  KEYBOARD_KEY_DOWN,
				  KEYBOARD_KEY_PGDOWN,
				  KEYBOARD_KEY_INS,
				  KEYBOARD_KEY_DEL,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_F11,
				  KEYBOARD_KEY_F12,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN };

const uint32_t uppercase[128] = { KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_ESC,
				  '!',
				  '@',
				  '#',
				  '$',
				  '%',
				  '^',
				  '&',
				  '*',
				  '(',
				  ')',
				  '_',
				  '+',
				  '\b',
				  '\t',
				  'Q',
				  'W',
				  'E',
				  'R',
				  'T',
				  'Y',
				  'U',
				  'I',
				  'O',
				  'P',
				  '{',
				  '}',
				  '\n',
				  KEYBOARD_KEY_CTRL,
				  'A',
				  'S',
				  'D',
				  'F',
				  'G',
				  'H',
				  'J',
				  'K',
				  'L',
				  ':',
				  '"',
				  '~',
				  KEYBOARD_KEY_LSHFT,
				  '|',
				  'Z',
				  'X',
				  'C',
				  'V',
				  'B',
				  'N',
				  'M',
				  '<',
				  '>',
				  '?',
				  KEYBOARD_KEY_RSHFT,
				  '*',
				  KEYBOARD_KEY_ALT,
				  ' ',
				  KEYBOARD_KEY_CAPS,
				  KEYBOARD_KEY_F1,
				  KEYBOARD_KEY_F2,
				  KEYBOARD_KEY_F3,
				  KEYBOARD_KEY_F4,
				  KEYBOARD_KEY_F5,
				  KEYBOARD_KEY_F6,
				  KEYBOARD_KEY_F7,
				  KEYBOARD_KEY_F8,
				  KEYBOARD_KEY_F9,
				  KEYBOARD_KEY_F10,
				  KEYBOARD_KEY_NUMLCK,
				  KEYBOARD_KEY_SCRLCK,
				  KEYBOARD_KEY_HOME,
				  KEYBOARD_KEY_UP,
				  KEYBOARD_KEY_PGUP,
				  '-',
				  KEYBOARD_KEY_LEFT,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_RIGHT,
				  '+',
				  KEYBOARD_KEY_END,
				  KEYBOARD_KEY_DOWN,
				  KEYBOARD_KEY_PGDOWN,
				  KEYBOARD_KEY_INS,
				  KEYBOARD_KEY_DEL,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_F11,
				  KEYBOARD_KEY_F12,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN,
				  KEYBOARD_KEY_UNKNOWN };

internal bool update_modifier_keys_state(enum PS2_scancode_set_1 scancode,
					 bool press);
internal void log_key_press(enum PS2_scancode_set_1 scancode);
void irq_1_handler(unused struct registers_t *regs);

void keyboard_init(void)
{
#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("initing keyboard input");
#endif

	CTRL_pressed = FALSE;
	SHIFT_pressed = FALSE;
	ALT_pressed = FALSE;

	irq_install_handler(1, &irq_1_handler);

#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("initing keyboard OK");
#endif
}

void irq_1_handler(unused struct registers_t *regs)
{
	char scancode = inb(PC_AC_KEYBOARD_CONTROLLER_PORT) & 0x7F;
	char press = inb(PC_AC_KEYBOARD_CONTROLLER_PORT) & 0x80; // down or up

	bool is_modifier = update_modifier_keys_state(scancode, press);

	if (!is_modifier) {
		log_key_press(scancode);
	}
}

internal bool update_modifier_keys_state(enum PS2_scancode_set_1 scancode,
					 bool press)
{
	switch (scancode) {
	case SC_LCTRL:
		CTRL_pressed = !press;
		return TRUE;
	case SC_LSHIFT:
	case SC_RSHIFT:
		SHIFT_pressed = !press;
		return TRUE;
	case SC_LALT:
		ALT_pressed = !press;
		return TRUE;
	default:
		break;
	}

	return FALSE;
}

internal void log_key_press(enum PS2_scancode_set_1 scancode)
{
	if (SHIFT_pressed) {
		printf("%c", uppercase[scancode]);
	}
	else {
		printf("%c", lowercase[scancode]);
	}
	return;
}
