import React, { useState, useEffect, useRef } from "react";
import {
    View,
    Text,
    StyleSheet,
    Alert,
    ActivityIndicator,
    TouchableOpacity,
} from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useRouter } from "expo-router";
import {
    CameraView,
    CameraType,
    BarcodeScanningResult,
    useCameraPermissions,
} from "expo-camera";
import * as FileSystem from "expo-file-system";

export default function AttendanceScreen() {
    const [permission, requestPermission] = useCameraPermissions();
    const [isLoading, setIsLoading] = useState(false);
    const [code, setCode] = useState("");
    const [step, setStep] = useState<"selfie" | "qr">("selfie");
    const router = useRouter();
    const cameraRef = useRef<CameraView>(null);

    useEffect(() => {
        (async () => {
            if (!permission?.granted) {
                await requestPermission();
            }

            // Get classroom code from storage
            try {
                const storedCode = await AsyncStorage.getItem("code");
                if (storedCode) {
                    setCode(storedCode);
                }
            } catch (error) {
                console.error("Error getting classroom code:", error);
            }
        })();
    }, [permission]);

    const takeSelfie = async () => {
        if (!cameraRef.current) return;

        setIsLoading(true);
        try {
            const photo = await cameraRef.current.takePictureAsync({
                quality: 0.7,
                base64: true,
                exif: false,
            });

            const token = await AsyncStorage.getItem("jwtToken");
            if (!token) {
                Alert.alert("Error", "Please log in to mark attendance");
                router.replace("/login");
                return;
            }

            // Create form data for the image
            const formData = new FormData();
            formData.append("selfie", {
                uri: photo.uri,
                type: "image/jpeg",
                name: "selfie.jpg",
            } as any);

            const response = await fetch(
                "http://159.89.19.111/airclass-api/upload-selfie",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "multipart/form-data",
                        Authorization: `Bearer ${token}`,
                    },
                    body: formData,
                }
            );

            const data = await response.json();

            if (response.ok) {
                // Move to QR scanning step
                setStep("qr");
            } else {
                Alert.alert("Error", data.message || "Failed to upload selfie");
            }
        } catch (error) {
            console.error("Error taking/uploading selfie:", error);
            Alert.alert("Error", "Failed to take or upload selfie");
        } finally {
            setIsLoading(false);
        }
    };

    const handleBarCodeScanned = async ({ data }: { data: string }) => {
        if (isLoading) return;

        setIsLoading(true);
        try {
            const token = await AsyncStorage.getItem("jwtToken");
            if (!token) {
                Alert.alert("Error", "Please log in to mark attendance");
                router.replace("/login");
                return;
            }

            const classroomId = await AsyncStorage.getItem("classroomId");
            if (!classroomId) {
                Alert.alert("Error", "No classroom selected");
                router.back();
                return;
            }

            const response = await fetch(
                "http://159.89.19.111/airclass-api/attendance",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Authorization: `Bearer ${token}`,
                    },
                    body: JSON.stringify({
                        code: data.trim(),
                        classroom_id: classroomId,
                    }),
                }
            );

            const responseData = await response.json();

            if (response.status === 201) {
                Alert.alert("Success", "Attendance marked successfully");
                router.back();
            } else if (response.status === 400) {
                Alert.alert(
                    "Invalid Code",
                    responseData.message ||
                        "The attendance code is invalid or expired"
                );
            } else if (response.status === 403) {
                Alert.alert(
                    "Not Allowed",
                    responseData.message ||
                        "You are not authorized to mark attendance or have already marked attendance for today"
                );
            } else {
                Alert.alert(
                    "Error",
                    responseData.message || "Failed to mark attendance"
                );
            }
        } catch (error) {
            console.error("Error marking attendance:", error);
            Alert.alert("Error", "Network error. Please try again.");
        } finally {
            setIsLoading(false);
        }
    };

    if (!permission?.granted) {
        return (
            <View style={styles.container}>
                <Text style={styles.errorText}>No access to camera</Text>
                <TouchableOpacity
                    style={styles.button}
                    onPress={() => router.back()}
                >
                    <Text style={styles.buttonText}>Go Back</Text>
                </TouchableOpacity>
            </View>
        );
    }

    return (
        <View style={styles.container}>
            {step === "selfie" ? (
                <>
                    <CameraView
                        ref={cameraRef}
                        style={styles.camera}
                        facing="front"
                    >
                        <View style={styles.overlay}>
                            <View style={styles.selfieGuide} />
                            {isLoading && (
                                <View style={styles.loadingOverlay}>
                                    <ActivityIndicator
                                        size="large"
                                        color="#fff"
                                    />
                                </View>
                            )}
                        </View>
                    </CameraView>
                    <View style={styles.footer}>
                        <Text style={styles.footerText}>
                            Take a selfie to verify your attendance
                        </Text>
                        <TouchableOpacity
                            style={styles.captureButton}
                            onPress={takeSelfie}
                            disabled={isLoading}
                        >
                            <View style={styles.captureButtonInner} />
                        </TouchableOpacity>
                    </View>
                </>
            ) : (
                <>
                    <CameraView
                        style={styles.camera}
                        facing="back"
                        onBarcodeScanned={handleBarCodeScanned}
                        barcodeScannerSettings={{
                            barcodeTypes: ["qr"],
                        }}
                    >
                        <View style={styles.overlay}>
                            <View style={styles.scanArea} />
                            {isLoading && (
                                <View style={styles.loadingOverlay}>
                                    <ActivityIndicator
                                        size="large"
                                        color="#fff"
                                    />
                                </View>
                            )}
                        </View>
                    </CameraView>
                    <View style={styles.footer}>
                        <Text style={styles.footerText}>
                            Scan QR Code to Mark Attendance
                        </Text>
                    </View>
                </>
            )}
        </View>
    );
}

const styles = StyleSheet.create({
    container: {
        flex: 1,
        backgroundColor: "#000",
    },
    camera: {
        flex: 1,
    },
    overlay: {
        flex: 1,
        backgroundColor: "rgba(0,0,0,0.5)",
        justifyContent: "center",
        alignItems: "center",
    },
    scanArea: {
        width: 250,
        height: 250,
        borderWidth: 2,
        borderColor: "#fff",
        backgroundColor: "transparent",
    },
    selfieGuide: {
        width: 250,
        height: 250,
        borderWidth: 2,
        borderColor: "#fff",
        borderRadius: 125,
        backgroundColor: "transparent",
    },
    loadingOverlay: {
        ...StyleSheet.absoluteFillObject,
        backgroundColor: "rgba(0,0,0,0.7)",
        justifyContent: "center",
        alignItems: "center",
    },
    footer: {
        position: "absolute",
        bottom: 0,
        left: 0,
        right: 0,
        backgroundColor: "rgba(0,0,0,0.7)",
        padding: 20,
        alignItems: "center",
    },
    footerText: {
        color: "#fff",
        fontSize: 16,
        textAlign: "center",
        marginBottom: 20,
    },
    errorText: {
        color: "#fff",
        fontSize: 18,
        textAlign: "center",
        marginBottom: 20,
    },
    button: {
        backgroundColor: "#1E3A8A",
        paddingVertical: 12,
        paddingHorizontal: 24,
        borderRadius: 8,
    },
    buttonText: {
        color: "#fff",
        fontSize: 16,
        fontWeight: "600",
    },
    captureButton: {
        width: 70,
        height: 70,
        borderRadius: 35,
        backgroundColor: "rgba(255, 255, 255, 0.3)",
        justifyContent: "center",
        alignItems: "center",
    },
    captureButtonInner: {
        width: 60,
        height: 60,
        borderRadius: 30,
        backgroundColor: "#fff",
    },
});
