import React, { useState } from "react";
import {
    View,
    TouchableOpacity,
    StyleSheet,
    Alert,
    ActivityIndicator,
} from "react-native";
import { Ionicons } from "@expo/vector-icons";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { API_BASE_URL } from "../constants/api";

export function SlideControl() {
    const [isLoading, setIsLoading] = useState(false);

    const sendSlideControlRequest = async (action: "next" | "previous") => {
        try {
            setIsLoading(true);
            const token = await AsyncStorage.getItem("jwtToken");
            const classroomId = await AsyncStorage.getItem("classroomId");
            const slideId = await AsyncStorage.getItem("currentSlideId");

            if (!token || !classroomId || !slideId) {
                Alert.alert(
                    "Error",
                    "Missing required data. Please try again."
                );
                return;
            }

            const response = await fetch(
                `${API_BASE_URL}/airclass-api/slide-control`,
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Authorization: `Bearer ${token}`,
                    },
                    body: JSON.stringify({
                        slide_id: slideId,
                        classroom_id: classroomId,
                        action: action,
                    }),
                }
            );

            const data = await response.json();

            if (!response.ok) {
                throw new Error(data.message || "Failed to control slide");
            }

            if (data.status === false) {
                throw new Error(data.message || "Failed to control slide");
            }
        } catch (error) {
            console.error("Error controlling slide:", error);
            Alert.alert(
                "Error",
                error instanceof Error
                    ? error.message
                    : "Failed to control slide"
            );
        } finally {
            setIsLoading(false);
        }
    };

    const handleNext = () => {
        sendSlideControlRequest("next");
    };

    const handlePrevious = () => {
        sendSlideControlRequest("previous");
    };

    return (
        <View style={styles.container}>
            <TouchableOpacity
                style={[styles.button, isLoading && styles.buttonDisabled]}
                onPress={handlePrevious}
                disabled={isLoading}
            >
                {isLoading ? (
                    <ActivityIndicator color="#fff" />
                ) : (
                    <Ionicons name="chevron-back" size={32} color="#fff" />
                )}
            </TouchableOpacity>
            <TouchableOpacity
                style={[styles.button, isLoading && styles.buttonDisabled]}
                onPress={handleNext}
                disabled={isLoading}
            >
                {isLoading ? (
                    <ActivityIndicator color="#fff" />
                ) : (
                    <Ionicons name="chevron-forward" size={32} color="#fff" />
                )}
            </TouchableOpacity>
        </View>
    );
}

const styles = StyleSheet.create({
    container: {
        flexDirection: "row",
        justifyContent: "space-between",
        padding: 16,
        gap: 16,
        backgroundColor: "#F3F4F6",
        borderRadius: 12,
        marginHorizontal: 16,
    },
    button: {
        flex: 1,
        backgroundColor: "#2563EB",
        paddingVertical: 16,
        paddingHorizontal: 24,
        borderRadius: 8,
        alignItems: "center",
        justifyContent: "center",
        shadowColor: "#000",
        shadowOffset: {
            width: 0,
            height: 2,
        },
        shadowOpacity: 0.1,
        shadowRadius: 3,
        elevation: 3,
    },
    buttonDisabled: {
        opacity: 0.7,
    },
});
