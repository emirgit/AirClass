import { useState, useEffect, useCallback } from "react";

interface WebSocketMessage {
    type: "attendance" | "speakRequest" | "slideControl";
    studentId?: string;
    command?: "next" | "previous";
}

export function useWebSocket() {
    const [socket, setSocket] = useState<WebSocket | null>(null);
    const [isConnected, setIsConnected] = useState(false);

    useEffect(() => {
        // Replace with your WebSocket server URL
        const ws = new WebSocket("ws://your-websocket-server-url");

        ws.onopen = () => {
            setIsConnected(true);
        };

        ws.onclose = () => {
            setIsConnected(false);
        };

        ws.onerror = (error) => {
            console.error("WebSocket error:", error);
            setIsConnected(false);
        };

        setSocket(ws);

        return () => {
            ws.close();
        };
    }, []);

    const sendMessage = useCallback(
        (message: WebSocketMessage) => {
            if (socket && isConnected) {
                socket.send(JSON.stringify(message));
            }
        },
        [socket, isConnected]
    );

    return {
        sendMessage,
        isConnected,
    };
}
