/* Programme de test
 * pour LLMP16asm
 */

mov r0 0
mov r1 0xFF

loop:
	vstr r0 r1
	inc r0
	jmp [loop]
