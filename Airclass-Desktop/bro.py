import socket
import asyncio
import websockets
import json
import time
import netifaces # Gerekli: pip install netifaces

# UDP Ayarları
UDP_PORT = 9999
UDP_BROADCAST_MESSAGE = "raspberry_discovery" # C++ donanımının gönderdiği mesaj
BUFFER_SIZE = 1024

# WebSocket Ayarları
WEBSOCKET_HOST = "0.0.0.0"  # Tüm arayüzlerden dinle
WEBSOCKET_PORT = 8080       # C++ donanımının Qt'ye bağlanacağı port

# Aktif WebSocket bağlantılarını saklamak için
connected_clients = set()

def get_local_ip_for_client(client_address_tuple):
    """
    İstemci ile aynı alt ağda olan yerel IP adresini bulur.
    Bu fonksiyon, Qt kodunuzdaki UdpDiscoveryServer::getLocalIPv4ForSender'a benzer.
    """
    client_ip_str = client_address_tuple[0]
    # print(f"[get_local_ip_for_client] İstemci IP: {client_ip_str}")

    for iface_name in netifaces.interfaces():
        # print(f"[get_local_ip_for_client] Arayüz kontrol ediliyor: {iface_name}")
        try:
            addrs = netifaces.ifaddresses(iface_name)
            if netifaces.AF_INET in addrs:
                for link_addr in addrs[netifaces.AF_INET]:
                    ip_addr = link_addr.get('addr')
                    netmask = link_addr.get('netmask')
                    # print(f"[get_local_ip_for_client] Bulunan IP: {ip_addr}, Netmask: {netmask}")
                    if ip_addr and netmask:
                        # Basit bir alt ağ kontrolü. Daha gelişmiş bir kontrol gerekebilir.
                        # İstemci IP'sinin ilk N biti ile yerel IP'nin ilk N bitini karşılaştır.
                        # Örnek: 255.255.255.0 için ilk 3 oktet aynı olmalı.
                        client_octets = client_ip_str.split('.')
                        local_octets = ip_addr.split('.')
                        mask_octets = netmask.split('.')
                        
                        match = True
                        for i in range(4):
                            if (int(client_octets[i]) & int(mask_octets[i])) != \
                               (int(local_octets[i]) & int(mask_octets[i])):
                                match = False
                                break
                        if match:
                            # print(f"[get_local_ip_for_client] Eşleşen IP bulundu: {ip_addr} for client {client_ip_str}")
                            return ip_addr
        except ValueError as e:
            print(f"[get_local_ip_for_client] Arayüz {iface_name} işlenirken hata: {e}")
        except Exception as e:
            print(f"[get_local_ip_for_client] Arayüz {iface_name} için bilinmeyen hata: {e}")
            
    print(f"[get_local_ip_for_client] {client_ip_str} için uygun yerel IP bulunamadı.")
    # Varsayılan olarak ilk uygun olmayan döngü dışı IP'yi döndür, ya da daha iyisi bir tane seçin.
    # Bu kısım ağ yapınıza göre özelleştirilmelidir.
    # Genellikle ilk IPv4 adresi yeterli olur eğer karmaşık bir ağ yapınız yoksa.
    for iface_name in netifaces.interfaces():
        addrs = netifaces.ifaddresses(iface_name)
        if netifaces.AF_INET in addrs:
            ip_addr = addrs[netifaces.AF_INET][0].get('addr')
            if ip_addr and not ip_addr.startswith("127."):
                 print(f"[get_local_ip_for_client] Varsayılan IP olarak {ip_addr} kullanılıyor.")
                 return ip_addr
    return "127.0.0.1" # Hiçbir şey bulunamazsa localhost


async def udp_discovery_server():
    """
    UDP broadcast mesajlarını dinler ve yanıtlar.
    """
    print(f"[UDP Sunucu] {UDP_PORT} portunda dinleme başlatıldı...")
    
    # IPv4 için UDP soketi oluştur
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Windows'ta SO_BROADCAST gerekmeyebilir, ancak Linux/macOS için genellikle gereklidir.
    # try:
    #     sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    # except OSError as e:
    #     print(f"[UDP Sunucu] SO_BROADCAST ayarlanamadı (Windows'ta normaldir): {e}")

    try:
        sock.bind(("", UDP_PORT)) # Tüm arayüzlerden gelen yayınları dinle
        print(f"[UDP Sunucu] {UDP_PORT} portuna başarıyla bağlandı.")
    except OSError as e:
        print(f"[UDP Sunucu] {UDP_PORT} portuna bağlanırken hata: {e}")
        print("[UDP Sunucu] Bu port başka bir uygulama tarafından kullanılıyor olabilir.")
        print("[UDP Sunucu] Qt uygulamanızın UDP sunucusunu testten önce kapattığınızdan emin olun.")
        return

    while True:
        try:
            print(f"[UDP Sunucu] '{UDP_BROADCAST_MESSAGE}' mesajı bekleniyor...")
            data, addr = sock.recvfrom(BUFFER_SIZE) # Veri ve gönderenin adresini al
            message = data.decode('utf-8')
            print(f"[UDP Sunucu] '{addr}' adresinden mesaj alındı: {message}")

            if message == UDP_BROADCAST_MESSAGE:
                # Qt uygulamasının beklediği IP adresini bul ve gönder
                response_ip = get_local_ip_for_client(addr)
                if response_ip:
                    print(f"[UDP Sunucu] '{UDP_BROADCAST_MESSAGE}' alındı. Yanıt olarak IP ({response_ip}) gönderiliyor: {addr}")
                    sock.sendto(response_ip.encode('utf-8'), addr)
                else:
                    print(f"[UDP Sunucu] Yanıtlanacak uygun yerel IP bulunamadı.")
            else:
                print(f"[UDP Sunucu] Beklenmeyen mesaj alındı: {message}")
        except ConnectionResetError:
            print(f"[UDP Sunucu] İstemci {addr} bağlantıyı sıfırladı.")
        except Exception as e:
            print(f"[UDP Sunucu] Bir hata oluştu: {e}")
            await asyncio.sleep(1) # Hata durumunda kısa bir bekleme


# async def websocket_handler(websocket): # ESKİ YANLIŞ HALİ
async def websocket_handler(websocket, path): # YENİ DOĞRU HALİ (path argümanı eklendi)
    """
    Yeni WebSocket bağlantılarını yönetir.
    """
    client_address = websocket.remote_address
    # 'path' değişkenini kullanmasanız bile fonksiyon imzasında olması gerekir.
    print(f"[WebSocket Sunucusu] Yeni bağlantı: {client_address} (Path: {path})")
    connected_clients.add(websocket)
    
    try:
        # İlk mesajı al (genellikle identify veya register mesajı)
        initial_message_str = await websocket.recv()
        print(f"[WebSocket Sunucusu] {client_address} adresinden ilk mesaj: {initial_message_str}")
        try:
            initial_message = json.loads(initial_message_str)
            if initial_message.get("type") == "identify" and initial_message.get("role") == "desktop":
                room_id = initial_message.get("roomId", "BilinmeyenOda")
                session_id = initial_message.get("sessionId", "BilinmeyenOturum")
                print(f"[WebSocket Sunucusu] Masaüstü istemcisi {client_address} tanımlandı. Oda: {room_id}, Oturum: {session_id}")
                
                # İsteğe bağlı: Başarılı kayıt mesajı gönder
                await websocket.send(json.dumps({"type": "registration_success", "client_id": str(client_address), "message": "Desktop client registered"}))

            elif initial_message.get("register") == "hardware": # C++ donanımının gönderdiği format
                 client_id = initial_message.get("id", "BilinmeyenDonanim")
                 print(f"[WebSocket Sunucusu] Donanım istemcisi {client_address} kayıt oldu. ID: {client_id}")
                 await websocket.send(json.dumps({"type": "registration_success", "client_id": client_id, "message": "Hardware client registered"}))
            else:
                print(f"[WebSocket Sunucu] {client_address} adresinden bilinmeyen ilk mesaj formatı.")

        except json.JSONDecodeError:
            print(f"[WebSocket Sunucusu] {client_address} adresinden gelen ilk mesaj JSON formatında değil.")
        except Exception as e:
            print(f"[WebSocket Sunucusu] {client_address} ilk mesaj işlenirken hata: {e}")


        # Buradan sonra istemciden gelen diğer mesajları dinleyebilir veya periyodik veri gönderebilirsiniz.
        gesture_counter = 0
        while True:
            # İstemciden mesaj beklemek için (veya periyodik gönderme yapmak için)
            try:
                # Periyodik mesaj gönderme örneği (isterseniz yorum satırını kaldırın)
                # await asyncio.sleep(5) 
                # gesture_payload = {
                #     "type": "gesture",
                #     "gesture_type": "NEXT_SLIDE",
                #     "client_id": "python_mock_server",
                #     "timestamp": int(time.time() * 1000)
                # }
                # await websocket.send(json.dumps(gesture_payload))
                # print(f"[WebSocket Sunucusu] {client_address} adresine mesaj gönderildi: {gesture_payload['gesture_type']}")
                
                # İstemciden mesaj bekleme
                message_str = await asyncio.wait_for(websocket.recv(), timeout=60.0) # Zaman aşımını artırabilirsiniz
                print(f"[WebSocket Sunucusu] {client_address} adresinden mesaj alındı: {message_str}")
                # Gelen mesaja göre bir yanıt gönderebilirsiniz
                # await websocket.send(json.dumps({"response_to": message_str, "status": "received"}))

            except asyncio.TimeoutError:
                # Zaman aşımı durumunda bağlantıyı kontrol etmek için ping gönderebilirsiniz
                try:
                    pong_waiter = await websocket.ping()
                    await asyncio.wait_for(pong_waiter, timeout=10)
                    # print(f"[WebSocket Sunucusu] {client_address} adresine Ping başarılı.")
                except asyncio.TimeoutError:
                    print(f"[WebSocket Sunucusu] {client_address} adresine Ping yanıtı gelmedi, bağlantı kopmuş olabilir.")
                    break # İç döngüden çık, bağlantı kapanacak
                except websockets.exceptions.ConnectionClosed:
                    print(f"[WebSocket Sunucusu] {client_address} bağlantısı ping sırasında kapandı.")
                    break
            except websockets.exceptions.ConnectionClosed:
                print(f"[WebSocket Sunucusu] {client_address} bağlantısı mesaj beklenirken/gönderilirken kapandı.")
                break
            except Exception as e:
                print(f"[WebSocket Sunucusu] {client_address} ile iletişimde hata: {e}")
                break
            # gesture_counter += 1 # Eğer periyodik gönderme yapıyorsanız
            
    except websockets.exceptions.ConnectionClosedOK:
        print(f"[WebSocket Sunucusu] {client_address} bağlantısı düzgün kapatıldı.")
    except websockets.exceptions.ConnectionClosedError as e:
        print(f"[WebSocket Sunucusu] {client_address} bağlantısı hatayla kapandı: {e}")
    except Exception as e:
        print(f"[WebSocket Sunucusu] {client_address} ile ilgili bilinmeyen hata: {e}")
    finally:
        print(f"[WebSocket Sunucusu] {client_address} bağlantısı sonlandırıldı.")
        if websocket in connected_clients:
            connected_clients.remove(websocket)


async def main():
    # UDP sunucusunu ayrı bir görev olarak başlat
    # Qt uygulamanız UDP sunucusunu çalıştırdığı için Python'da UDP sunucusunu başlatmıyoruz.
    # Sadece C++ donanımının yaptığı gibi broadcast gönderip yanıt alacağız.
    
    # C++ Donanımının yaptığı UDP Discovery'yi simüle et
    hardware_client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hardware_client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    hardware_client_socket.settimeout(5.0) # 5 saniye zaman aşımı

    desktop_ip_for_hardware = None
    try:
        print(f"[Donanım Simülasyonu] UDP Broadcast gönderiliyor: '{UDP_BROADCAST_MESSAGE}' -> 255.255.255.255:{UDP_PORT}")
        hardware_client_socket.sendto(UDP_BROADCAST_MESSAGE.encode('utf-8'), ('<broadcast>', UDP_PORT))
        
        print("[Donanım Simülasyonu] Qt uygulamasından UDP yanıtı bekleniyor...")
        data, server_addr = hardware_client_socket.recvfrom(BUFFER_SIZE)
        desktop_ip_for_hardware = data.decode('utf-8')
        print(f"[Donanım Simülasyonu] Qt uygulamasından IP alındı: {desktop_ip_for_hardware} (Sunucu: {server_addr})")
        
        # WebSocket sunucusunu bu keşfedilen IP'de başlat
        # Ancak pratikte 0.0.0.0 kullanmak daha kolaydır, tüm arayüzlerden dinler.
        # WebSocket istemcisi (Qt) bu IP'ye bağlanacaktır.
        # WebSocket sunucusu ise yine kendi üzerinde (0.0.0.0 veya belirli bir IP) çalışır.
        # Bu Python betiği Qt uygulamasının bağlanacağı WebSocket sunucusu olduğu için,
        # Qt uygulamasının bu Python betiğinin çalıştığı makinenin IP'sine bağlanması gerekir.
        # Qt'nin aldığı 'desktop_ip_for_hardware' aslında Qt'nin kendi IP'si olacak.
        # Dolayısıyla, Qt bu Python betiğinin IP'sine bağlanmalı.

        global WEBSOCKET_HOST # WebSocket hostunu globalden al
        # Eğer Qt'nin keşfettiği IP'yi kullanmak isterseniz, o IP'de dinlemelisiniz.
        # Ancak bu Python scripti Qt uygulamasının bağlanacağı bir sunucu.
        # Dolayısıyla Qt'nin bağlanacağı IP, bu scriptin çalıştığı makinenin IP'si olmalı.
        # UDP discovery, Qt'nin kendi IP'sini (veya donanımın bağlanacağı IP'yi) bulmasını sağlar.
        # Bu script bir donanım DEĞİL, Qt'nin bağlanacağı bir sunucu olduğu için,
        # Qt'nin bu scriptin IP'sini bilmesi gerekir. UDP discovery bu senaryoda Qt'ye kendi IP'sini verir.
        # Eğer bu script bir donanım olsaydı, Qt'nin IP'sini UDP ile alır ve ona bağlanırdı.
        # Mevcut durumda Qt, UDP ile kendi IP'sini öğrenip bu IP'deki WebSocket sunucusuna (bu script) bağlanmaya çalışacak.

        print(f"\n[WebSocket Sunucusu] Başlatılıyor: ws://{WEBSOCKET_HOST}:{WEBSOCKET_PORT}")
        print(f"[WebSocket Sunucusu] Qt uygulamanızın bağlanması gereken adres: ws://{desktop_ip_for_hardware}:{WEBSOCKET_PORT} (eğer bu script Qt ile aynı makinede ise)")
        print(f"[WebSocket Sunucusu] Ya da Qt uygulamanız bu makinenin ağdaki IP'sine (ws://<bu_makinenin_IPsi>:{WEBSOCKET_PORT}) bağlanmalı.")

    except socket.timeout:
        print("[Donanım Simülasyonu] Qt uygulamasından UDP yanıtı alınamadı (zaman aşımı).")
        print(f"[WebSocket Sunucusu] Varsayılan ws://{WEBSOCKET_HOST}:{WEBSOCKET_PORT} adresinde başlatılıyor.")
    except Exception as e:
        print(f"[Donanım Simülasyonu] UDP discovery sırasında hata: {e}")
        print(f"[WebSocket Sunucusu] Varsayılan ws://{WEBSOCKET_HOST}:{WEBSOCKET_PORT} adresinde başlatılıyor.")
    finally:
        hardware_client_socket.close()

    # WebSocket sunucusunu başlat
    start_server = await websockets.serve(websocket_handler, WEBSOCKET_HOST, WEBSOCKET_PORT)
    print(f"[WebSocket Sunucusu] ws://{WEBSOCKET_HOST}:{WEBSOCKET_PORT} adresinde dinleniyor.")
    
    await start_server.wait_closed()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n[Ana Program] Sunucu kapatılıyor...")
    except OSError as e:
        if e.errno == 98 or e.errno == 10048: # Address already in use
             print(f"\n[HATA] Port {WEBSOCKET_PORT} (WebSocket) veya {UDP_PORT} (UDP) zaten kullanılıyor.")
             print("Lütfen başka bir uygulamanın bu portları kullanmadığından emin olun veya script'i farklı portlarla çalıştırın.")
        else:
            print(f"\n[Ana Program] Bir işletim sistemi hatası oluştu: {e}")
    except Exception as e:
        print(f"\n[Ana Program] Beklenmedik bir hata oluştu: {e}")