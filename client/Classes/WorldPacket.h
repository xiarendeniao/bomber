#ifndef WOWSERVER_WORLDPACKET_H
#define WOWSERVER_WORLDPACKET_H

#include "ByteBuffer.h"

class WorldPacket : public ByteBuffer
{
public:
    __inline WorldPacket() : ByteBuffer(), m_opcode(0) { }
    __inline WorldPacket(uint16 opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
    __inline WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) { }
    __inline WorldPacket(const WorldPacket &packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}

    //! Clear packet and set opcode all in one mighty blow
    __inline void Initialize(uint16 opcode )
    {
        clear();
        m_opcode = opcode;
    }

    __inline uint16 GetOpcode() const { return m_opcode; }
    __inline void SetOpcode(uint16 opcode) { m_opcode = opcode; }

	void AppendPacketData(const WorldPacket& data) {
		uint16 count = data.size();
		if(count) {
			append((uint8 *)data.contents(), count);
		}
	}

	void AppendPacketData(const WorldPacket& data, uint16 startPos){
		uint16 count = data.size();
		if (count > startPos)
			append((uint8 *)data.contents()+startPos, count-startPos);
	}

	void AppendPacket(const WorldPacket& data) {
		uint16 count = data.size();
		*this << (uint16)(count+2);
		*this << (uint16)data.m_opcode;
		if(count) {
			append((uint8 *)data.contents(), count);
		}
	}

	WorldPacket BuildPVPWorldPacket(const WorldPacket& data)
	{
		WorldPacket pack;
		pack.SetOpcode(data.GetOpcode());
		if (data.size()>1){
			pack.append((uint8 *)data.contents()+2, pack.size()-2);
		}
		return pack;
	}

	void ReadPacket(WorldPacket& data) {
		uint16 len;
		*this >> len;
		if(len >= 2) {
			uint16 op;
			*this >> op;
			data.Initialize(op);
			len -= 2;
			if(len) {
				vector<uint8> val;
				val.resize(len);
				read((uint8*)&val[0], len);
				data.append((uint8 *)&val[0], len);
			}
		}
	}

protected:
    uint16 m_opcode;
};


#endif
