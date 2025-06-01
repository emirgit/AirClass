import React, { useState, useRef } from "react";
import {
    View,
    TouchableOpacity,
    Text,
    StyleSheet,
    Alert,
    ActivityIndicator,
    Modal,
} from "react-native";
import { CameraView, useCameraPermissions } from "expo-camera";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useRouter } from "expo-router";

interface MarkAttendanceProps {
    onAttendanceMarked?: () => void;
}

export function MarkAttendance({ onAttendanceMarked }: MarkAttendanceProps) {
    const [permission, requestPermission] = useCameraPermissions();
    const [isCameraOpen, setIsCameraOpen] = useState(false);
    const [isLoading, setIsLoading] = useState(false);
    const [step, setStep] = useState<"qr" | "selfie">("qr");
    const lastScannedCode = useRef<string | null>(null);
    const cameraRef = useRef<CameraView>(null);
    const router = useRouter();
    const [attendanceData, setAttendanceData] = useState<{
        token: string;
        classroomId: string;
    } | null>(null);

    const handleBarcodeScanned = async ({ data }: { data: string }) => {
        if (lastScannedCode.current === data || isLoading) {
            return;
        }
        lastScannedCode.current = data;
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
                // Attendance marked, now prompt for selfie
                setAttendanceData({ token, classroomId });
                setStep("selfie");
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

    const takeSelfie = async () => {
        if (!cameraRef.current || !attendanceData) return;
        setIsLoading(true);
        try {
            const photo = await cameraRef.current.takePictureAsync({
                quality: 0.7,
                base64: true,
                exif: false,
            });
            // Create form data for the image
            const formData = new FormData();
            formData.append("image", {
                uri: photo.uri,
                type: "image/jpeg",
                name: "selfie.jpg",
            } as any);
            formData.append("classroom_id", attendanceData.classroomId || "");
            const response = await fetch(
                "http://159.89.19.111/airclass-api/image",
                {
                    method: "POST",
                    headers: {
                        Authorization: `Bearer ${attendanceData.token}`,
                    },
                    body: formData,
                }
            );
            const text = await response.text();
            console.log("Upload response text:", text);
            let data;
            try {
                data = JSON.parse(text);
            } catch (e) {
                Alert.alert(
                    "Error",
                    "Server did not return valid JSON. See console for details."
                );
                return;
            }
            if (response.ok && data.status) {
                Alert.alert(
                    "Success",
                    "Attendance and selfie uploaded successfully"
                );
                setIsCameraOpen(false);
                setStep("qr");
                setAttendanceData(null);
                onAttendanceMarked?.();
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

    const handleOpenCamera = async () => {
        if (!permission?.granted) {
            const result = await requestPermission();
            if (!result.granted) {
                Alert.alert(
                    "Camera Permission Required",
                    "Please grant camera permission to mark attendance",
                    [{ text: "OK" }]
                );
                return;
            }
        }

        // Alert user about selfie requirement
        Alert.alert(
            "Attendance Verification",
            "To mark your attendance, you'll need to:\n\n1. Take a selfie for verification\n2. Scan the QR code\n\nThis helps ensure you are physically present in the classroom.",
            [
                {
                    text: "Cancel",
                    style: "cancel",
                },
                {
                    text: "Continue",
                    onPress: () => {
                        lastScannedCode.current = null;
                        setStep("qr");
                        setIsCameraOpen(true);
                    },
                },
            ]
        );
    };

    return (
        <View style={styles.container}>
            <TouchableOpacity
                style={[styles.button, styles.attendanceButton]}
                onPress={handleOpenCamera}
            >
                <Text style={styles.buttonText}>Mark Attendance</Text>
            </TouchableOpacity>

            {/* Fullscreen Camera Modal */}
            <Modal
                visible={isCameraOpen}
                animationType="slide"
                transparent={false}
                onRequestClose={() => {
                    setIsCameraOpen(false);
                    lastScannedCode.current = null;
                    setStep("qr");
                    setAttendanceData(null);
                }}
            >
                <View style={styles.cameraContainer}>
                    {step === "qr" ? (
                        <>
                            <View style={styles.cameraWrapper}>
                                <CameraView
                                    ref={cameraRef}
                                    style={StyleSheet.absoluteFill}
                                    facing="back"
                                    onBarcodeScanned={handleBarcodeScanned}
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
                            </View>
                            <View style={styles.cameraFooter}>
                                <Text style={styles.footerText}>
                                    Scan QR Code to Mark Attendance
                                </Text>
                            </View>
                        </>
                    ) : (
                        <>
                            <View style={styles.cameraWrapper}>
                                <CameraView
                                    ref={cameraRef}
                                    style={StyleSheet.absoluteFill}
                                    facing="front"
                                >
                                    <View style={styles.overlay}>
                                        <View style={styles.selfieGuide} />
                                        <Text style={styles.selfieGuideText}>
                                            Position your face within the circle
                                        </Text>
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
                            </View>
                            <View style={styles.cameraFooter}>
                                <Text style={styles.footerText}>
                                    Take a selfie to verify your attendance
                                </Text>
                                <View style={styles.buttonContainer}>
                                    <TouchableOpacity
                                        style={[
                                            styles.captureButton,
                                            isLoading && styles.buttonDisabled,
                                        ]}
                                        onPress={takeSelfie}
                                        disabled={isLoading}
                                    >
                                        <View
                                            style={styles.captureButtonInner}
                                        />
                                    </TouchableOpacity>
                                    <TouchableOpacity
                                        style={[
                                            styles.retakeButton,
                                            isLoading && styles.buttonDisabled,
                                        ]}
                                        onPress={() => {
                                            if (cameraRef.current) {
                                                cameraRef.current.resumePreview();
                                            }
                                        }}
                                        disabled={isLoading}
                                    >
                                        <Text style={styles.retakeButtonText}>
                                            Retake
                                        </Text>
                                    </TouchableOpacity>
                                </View>
                            </View>
                        </>
                    )}
                    <TouchableOpacity
                        style={styles.closeButton}
                        onPress={() => {
                            Alert.alert(
                                "Cancel Attendance",
                                "Are you sure you want to cancel marking attendance?",
                                [
                                    {
                                        text: "No, Continue",
                                        style: "cancel",
                                    },
                                    {
                                        text: "Yes, Cancel",
                                        onPress: () => {
                                            setIsCameraOpen(false);
                                            lastScannedCode.current = null;
                                            setStep("qr");
                                            setAttendanceData(null);
                                        },
                                    },
                                ]
                            );
                        }}
                    >
                        <Text style={styles.closeButtonText}>Close</Text>
                    </TouchableOpacity>
                </View>
            </Modal>
        </View>
    );
}

const styles = StyleSheet.create({
    container: {
        padding: 16,
        gap: 8,
    },
    button: {
        backgroundColor: "#4F46E5",
        paddingVertical: 12,
        paddingHorizontal: 24,
        borderRadius: 8,
        alignItems: "center",
    },
    attendanceButton: {
        backgroundColor: "#7C3AED",
    },
    buttonDisabled: {
        opacity: 0.7,
    },
    buttonText: {
        color: "#fff",
        fontSize: 16,
        fontWeight: "600",
    },
    cameraContainer: {
        position: "absolute",
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: "black",
        zIndex: 1000,
    },
    cameraWrapper: {
        flex: 1,
        position: "relative",
    },
    camera: {
        flex: 1,
    },
    overlay: {
        ...StyleSheet.absoluteFillObject,
        backgroundColor: "rgba(0,0,0,0.3)",
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
    cameraFooter: {
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
    closeButton: {
        position: "absolute",
        top: 40,
        right: 20,
        backgroundColor: "rgba(0,0,0,0.6)",
        padding: 12,
        borderRadius: 8,
    },
    closeButtonText: {
        color: "#fff",
        fontSize: 16,
        fontWeight: "600",
    },
    captureButton: {
        width: 70,
        height: 70,
        borderRadius: 35,
        backgroundColor: "rgba(79, 70, 229, 0.3)",
        justifyContent: "center",
        alignItems: "center",
    },
    captureButtonInner: {
        width: 60,
        height: 60,
        borderRadius: 30,
        backgroundColor: "#fff",
    },
    selfieGuideText: {
        color: "#fff",
        fontSize: 16,
        textAlign: "center",
        marginTop: 20,
        backgroundColor: "rgba(0,0,0,0.5)",
        padding: 8,
        borderRadius: 8,
    },
    buttonContainer: {
        flexDirection: "row",
        alignItems: "center",
        gap: 20,
    },
    retakeButton: {
        backgroundColor: "#4F46E5",
        paddingVertical: 12,
        paddingHorizontal: 24,
        borderRadius: 8,
    },
    retakeButtonText: {
        color: "#fff",
        fontSize: 16,
        fontWeight: "600",
    },
});