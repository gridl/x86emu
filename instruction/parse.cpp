#include <map>
#include "instruction/instruction.hpp"

uint8_t ParseInstr::parse_prefix(void){
	uint8_t code, chsz = 0;

	while(true){
		code = get_emu()->get_code8(0);
		switch(code){
			case 0x26:
				PRE_SEGMENT = ES;
				goto set_pre;
			case 0x2e:
				PRE_SEGMENT = CS;
				goto set_pre;
			case 0x36:
				PRE_SEGMENT = SS;
				goto set_pre;
			case 0x3e:
				PRE_SEGMENT = DS;
				goto set_pre;
			case 0x64:
				PRE_SEGMENT = FS;
				goto set_pre;
			case 0x65:
				PRE_SEGMENT = GS;
set_pre:			PREFIX = code;
				goto next;
			case 0x66:
				chsz |= CHSZ_OP;
				goto next;
			case 0x67:
				chsz |= CHSZ_AD;
next:				UPDATE_EIP(1);
				break;
			default:
				return chsz;
		}
	}
}

void ParseInstr::parse(void){
	parse_opcode();

	if(!chk.count(OPCODE)){
		DEBUG_MSG("\n");
		ERROR("no opecode : %x", OPCODE);
	}

	if(chk[OPCODE].modrm)
		parse_modrm_sib_disp();

	if(chk[OPCODE].imm32){
		IMM32 = get_emu()->get_code32(0);
		DEBUG_MSG("imm32:0x%08x ", IMM32);
		UPDATE_EIP(4);
	}
	else if(chk[OPCODE].imm16){
		IMM16 = get_emu()->get_code16(0);
		DEBUG_MSG("imm16:0x%04x ", IMM16);
		UPDATE_EIP(2);
	}
	else if(chk[OPCODE].imm8){
		IMM8 = (int8_t)get_emu()->get_code8(0);
		DEBUG_MSG("imm8:0x%02x ", IMM8);
		UPDATE_EIP(1);
	}
/*
	else if(chk[OPCODE].moffs8){
		MOFFS8 = (int8_t)get_emu()->get_code8(0);
		DEBUG_MSG("moffs8:0x%02x ", MOFFS8);
		UPDATE_EIP(1);
	}
	else if(chk[OPCODE].moffs){
		if(is_protected() ^ chsz_ad){
			MOFFS = get_emu()->get_code32(0);
			UPDATE_EIP(4);
		}
		else{
			MOFFS = get_emu()->get_code16(0);
			UPDATE_EIP(2);
		}
		DEBUG_MSG("moffs:0x%04x", MOFFS);
	}
*/
	if(chk[OPCODE].ptr16){
		PTR16 = get_emu()->get_code16(0);
		DEBUG_MSG("ptr16:0x%04x", PTR16);
		UPDATE_EIP(2);
	}


	DEBUG_MSG("\n");
}

void ParseInstr::parse_opcode(void){
	OPCODE = get_emu()->get_code8(0);
	UPDATE_EIP(1);
	
	// two byte opcode
	if(OPCODE == 0x0f){
		OPCODE = (OPCODE << 8) + get_emu()->get_code8(0);
		UPDATE_EIP(1);
	}

	if(is_protected())
		DEBUG_MSG("CS:%04x EIP:0x%04x opcode:%02x ", EMU->get_sgreg(CS), GET_EIP()-1, OPCODE);
	else
		DEBUG_MSG("CS:%04x  IP:0x%04x opcode:%02x ", EMU->get_sgreg(CS), GET_IP()-1, OPCODE);
}

void ParseInstr::parse_modrm_sib_disp(void){
	_MODRM = get_emu()->get_code8(0);
	UPDATE_EIP(1);

	DEBUG_MSG("[mod:0x%02x reg:0x%02x rm:0x%02x] ", MOD, REG, RM);

	if(is_protected() ^ chsz_ad)
		parse_modrm32();
	else
		parse_modrm16();
}

void ParseInstr::parse_modrm32(void){
	if (MOD != 3 && RM == 4) {
		_SIB = get_emu()->get_code8(0);
		UPDATE_EIP(1);
		DEBUG_MSG("[scale:0x%02x index:0x%02x base:0x%02x] ", SCALE, INDEX, BASE);
	}

	if (MOD == 2 || (MOD == 0 && RM == 5) || (MOD == 0 && BASE == 5)) {
		DISP32 = get_emu()->get_code32(0);
		UPDATE_EIP(4);
		DEBUG_MSG("disp32:0x%08x ", DISP32);
	}
	else if (MOD == 1) {
		DISP8 = (int8_t)get_emu()->get_code8(0);
		UPDATE_EIP(1);
		DEBUG_MSG("disp8:0x%02x ", DISP8);
	}
}

void ParseInstr::parse_modrm16(void){
	if ((MOD == 0 && RM == 6) || MOD == 2) {
		DISP16 = get_emu()->get_code32(0);
		UPDATE_EIP(2);
		DEBUG_MSG("disp16:0x%04x ", DISP16);
	}
	else if (MOD == 1) {
		DISP8 = (int8_t)get_emu()->get_code8(0);
		UPDATE_EIP(1);
		DEBUG_MSG("disp8:0x%02x ", DISP8);
	}
}
