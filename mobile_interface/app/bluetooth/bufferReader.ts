import { Buffer } from "buffer";

export class BufferReader {
  buffer: Buffer;
  offset: number;

  constructor(buffer: Buffer) {
    this.buffer = buffer;
    this.offset = 0;
  }

  // Functions for reading signed and unsigned char
  readChar() {
    const value = this.buffer.readInt8(this.offset);
    this.offset += 1;
    return value;
  }

  readUnsignedChar() {
    const value = this.buffer.readUInt8(this.offset);
    this.offset += 1;
    return value;
  }

  // Functions for reading signed and unsigned short (2 bytes)
  readShortLE() {
    const value = this.buffer.readInt16LE(this.offset);
    this.offset += 2;
    return value;
  }

  readShortBE() {
    const value = this.buffer.readInt16BE(this.offset);
    this.offset += 2;
    return value;
  }

  readUnsignedShortLE() {
    const value = this.buffer.readUInt16LE(this.offset);
    this.offset += 2;
    return value;
  }

  readUnsignedShortBE() {
    const value = this.buffer.readUInt16BE(this.offset);
    this.offset += 2;
    return value;
  }

  // Functions for reading signed and unsigned int (4 bytes)
  readIntLE() {
    const value = this.buffer.readInt32LE(this.offset);
    this.offset += 4;
    return value;
  }

  readIntBE() {
    const value = this.buffer.readInt32BE(this.offset);
    this.offset += 4;
    return value;
  }

  readUnsignedIntLE() {
    const value = this.buffer.readUInt32LE(this.offset);
    this.offset += 4;
    return value;
  }

  readUnsignedIntBE() {
    const value = this.buffer.readUInt32BE(this.offset);
    this.offset += 4;
    return value;
  }

  // Functions for reading signed and unsigned long long (8 bytes)
  readLongLongLE() {
    const high = this.buffer.readInt32LE(this.offset + 4);
    const low = this.buffer.readUInt32LE(this.offset);
    this.offset += 8;
    return low + high * 0x100000000;
  }

  readLongLongBE() {
    const high = this.buffer.readInt32BE(this.offset);
    const low = this.buffer.readUInt32BE(this.offset + 4);
    this.offset += 8;
    return low + high * 0x100000000;
  }

  readUnsignedLongLongLE() {
    const high = this.buffer.readUInt32LE(this.offset + 4);
    const low = this.buffer.readUInt32LE(this.offset);
    this.offset += 8;
    return low + high * 0x100000000;
  }

  readUnsignedLongLongBE() {
    const high = this.buffer.readUInt32BE(this.offset);
    const low = this.buffer.readUInt32BE(this.offset + 4);
    this.offset += 8;
    return low + high * 0x100000000;
  }

  // Functions for reading float and double (4 bytes and 8 bytes respectively)
  readFloatLE() {
    const value = this.buffer.readFloatLE(this.offset);
    this.offset += 4;
    return value;
  }

  readFloatBE() {
    const value = this.buffer.readFloatBE(this.offset);
    this.offset += 4;
    return value;
  }

  readDoubleLE() {
    const value = this.buffer.readDoubleLE(this.offset);
    this.offset += 8;
    return value;
  }

  readDoubleBE() {
    const value = this.buffer.readDoubleBE(this.offset);
    this.offset += 8;
    return value;
  }
}
