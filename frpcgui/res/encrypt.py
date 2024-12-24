# -*- coding: utf-8 -*-

# XOR 기반으로 EXE를 암호화하는 간단한 예제 스크립트

import os

def xor_data(data: bytes, key: bytes) -> bytes:
    enc_data = bytearray(data)
    key_len = len(key)
    for i in range(len(enc_data)):
        enc_data[i] ^= key[i % key_len]
    return bytes(enc_data)

def main():
    # 암호화할 대상 exe 파일명
    input_filename = "frpc.exe"
    # 암호화된 결과 파일명
    output_filename = "frpc.enc"
    # XOR 키 (20바이트)
    key = b"LfZA*j@v%&6ZnDF8xjr%"  # 20바이트 랜덤 키
    
    if not os.path.exists(input_filename):
        print(f"Error: {input_filename} 파일을 찾을 수 없습니다.")
        return
    
    with open(input_filename, "rb") as f:
        data = f.read()
    
    enc_data = xor_data(data, key)
    
    with open(output_filename, "wb") as f:
        f.write(enc_data)
    
    print(f"암호화 완료: {input_filename} -> {output_filename}")

if __name__ == "__main__":
    main()
