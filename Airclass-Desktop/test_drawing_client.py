import asyncio
import websockets
import json
import random
import math

async def send_drawing_data():
    uri = "ws://localhost:9090"  # WebSocket sunucu adresi güncellendi
    print(f"WebSocket sunucusuna bağlanmaya çalışılıyor: {uri}")
    
    try:
        async with websockets.connect(uri) as websocket:
            print("WebSocket sunucusuna bağlandı!")
            
            # Test için basit bir çizim verisi oluştur
            # Örnek: Bir daire çizelim
            center_x, center_y = 400, 300  # Merkez nokta
            radius = 100  # Yarıçap
            points = []
            
            # Daire için noktaları oluştur
            for angle in range(0, 361, 10):  # 10 derece aralıklarla
                x = center_x + radius * math.cos(math.radians(angle))
                y = center_y + radius * math.sin(math.radians(angle))
                points.append({"x": x, "y": y})

            # Çizim verilerini gönder
            for i, point in enumerate(points):
                data = {
                    "type": "drawing",
                    "x": point["x"],
                    "y": point["y"],
                    "isStart": i == 0,  # İlk nokta için true, diğerleri için false
                    "color": "red",
                    "width": 2
                }
                
                await websocket.send(json.dumps(data))
                print(f"Nokta gönderildi: {point}")
                await asyncio.sleep(0.05)  # Her nokta arasında 50ms bekle

            print("Çizim verisi gönderildi!")
    except Exception as e:
        print(f"Bağlantı hatası: {e}")

if __name__ == "__main__":
    asyncio.run(send_drawing_data()) 