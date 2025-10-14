import socket, json
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.bind(("", 40000))
print("Listening UDP 40000...")
while True:
    data, addr = s.recvfrom(4096)
    try:
        j = json.loads(data.decode("utf-8", errors="ignore"))
    except Exception:
        j = {"raw": data.decode("utf-8", errors="ignore")}
    print(addr, j)