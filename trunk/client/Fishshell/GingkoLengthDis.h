#ifndef __GINGKO_LENGTH_DIS_H__
#define __GINGKO_LENGTH_DIS_H__

#define SIZE16               0          // 16-bit Operand/Address
#define SIZE32               1          // 32-bit Operand/Address

#define RETFOUND             1          // Warning: RET found
#define JMPFOUND             2          // Warning: JMP found
#define CALLFOUND            4          // Warning: CALL found
#define ABSADDRFOUND         8          // Warning: Absolute addressing found

#define BYTE1                0x000000   // 1 byte opcode
#define BYTE2                0x000001   // 2 byte opcode
#define WORD8                0x000001   // opcode + byte (port, ...)
#define IMM8                 0x000001   // opcode + imm8
#define PREFIX               0x000002   // prefix opcode
#define OPSIZE               0x000004   // opcode 0x66 (operand size)
#define ADDRSIZE             0x000008   // opcode 0x67 (address size)
#define OPCODE0F             0x000010   // opcode 0x0F
#define WORD16               0x000020   // opcode + word16 (16-bit displacement, ...)
#define IMM1632              0x000040   // immediate data (16/32 bits)
#define ADDR1632             0x000080   // displacement (16/32 bits)
#define MODRM                0x000100   // mod r/m (+ sib) byte
#define FPOINT               0x000100   // Floting-point instructions
#define INT                  0x000200   // INT n (check special case of INT 0x20)
#define JMP8                 0x000400   // JMP disp8 (0-7F: forward, 80-FF: backward)
#define JMPOFFSET            0x000800   // JMP full offset (16/32 bits)
#define CALLOFFSET           0x001000   // CALL full offset (16/32 bits)
#define JMPOFFSEL            0x002000   // JMP Offset(16/32) + Selector(16)
#define CALLOFFSEL           0x004000   // CALL Offset(16/32) + Selector(16)
#define OPCODEF6             0x008000   // opcode 0xF6
#define OPCODEF7             0x010000   // opcode 0xF7
#define OPCODEFF             0x020000   // opcode 0xFF
#define RET                  0x040000   // RET/RETF
#define OPCODEAE             0x080000   // opcode 0x0F,0xAE
#define OPCODE0F0F           0x100000   // opcode 0x0F,0x0F


#endif
