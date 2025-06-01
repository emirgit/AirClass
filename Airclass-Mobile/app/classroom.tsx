import React, { useState, useEffect } from "react";
import {
    View,
    Text,
    StyleSheet,
    TextInput,
    TouchableOpacity,
    ActivityIndicator,
    Alert,
} from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useRouter } from "expo-router";

export default function ClassroomEntryScreen() {
    const router = useRouter();
    const [userInfo, setUserInfo] = useState<any>(null);
    const [isLoading, setIsLoading] = useState(true);
    const [classCode, setClassCode] = useState("");
    const [isJoining, setIsJoining] = useState(false);

    useEffect(() => {
        const checkAuth = async () => {
            console.log("Classroom: Checking authentication...");
            const token = await AsyncStorage.getItem("jwtToken");
            console.log("Classroom: Token exists?", !!token);
            if (!token) {
                console.log("Classroom: No token found, redirecting to login");
                router.replace("/login");
                return;
            }

            // Get user info from AsyncStorage instead of API
            try {
                const userEmail = await AsyncStorage.getItem("userEmail");
                const userName = await AsyncStorage.getItem("userName");
                if (userEmail && userName) {
                    setUserInfo({ email: userEmail, name: userName });
                }
            } catch (err) {
                console.error(
                    "Classroom: Error getting user info from storage:",
                    err
                );
            } finally {
                setIsLoading(false);
            }
        };
        checkAuth();
    }, []);

    const handleEnterClass = async () => {
        if (!classCode.trim()) return;

        setIsJoining(true);
        try {
            const token = await AsyncStorage.getItem("jwtToken");
            console.log("Verifying classroom code:", classCode.trim());
            const response = await fetch(
                "http://159.89.19.111/airclass-api/classroom/join",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Authorization: `Bearer ${token}`,
                    },
                    body: JSON.stringify({ code: classCode.trim() }),
                }
            );

            const data = await response.json();
            console.log("Classroom join response:", data);

            if (response.ok && data.status) {
                console.log("Classroom code valid, joining classroom...");
                try {
                    await AsyncStorage.setItem("code", classCode.trim());
                    // Store the classroom ID from the response
                    if (data.data?.classroom?.id) {
                        await AsyncStorage.setItem(
                            "classroomId",
                            data.data.classroom.id.toString()
                        );
                        console.log(
                            "Classroom ID stored:",
                            data.data.classroom.id
                        );
                    }
                    console.log("Classroom code stored in AsyncStorage");
                    console.log("Navigating to classroom interface...");
                    router.replace({
                        pathname: "/classroom-interface",
                        params: {
                            code: classCode.trim(),
                            timestamp: new Date().getTime(),
                        },
                    });
                } catch (storageError) {
                    console.error(
                        "Error storing classroom data:",
                        storageError
                    );
                    Alert.alert(
                        "Warning",
                        "Classroom joined but failed to save data locally"
                    );
                    // Still proceed with navigation even if storage fails
                    console.log(
                        "Navigating to classroom interface (after storage error)..."
                    );
                    router.replace({
                        pathname: "/classroom-interface",
                        params: {
                            code: classCode.trim(),
                            timestamp: new Date().getTime(),
                        },
                    });
                }
            } else {
                Alert.alert(
                    "Invalid Code",
                    data.message ||
                        "This classroom code is invalid or the classroom is not open"
                );
            }
        } catch (err) {
            console.error("Error joining classroom:", err);
            Alert.alert("Error", "Failed to join classroom. Please try again.");
        }
        setIsJoining(false);
    };

    if (isLoading) {
        return (
            <View style={styles.container}>
                <ActivityIndicator size="large" color="#0000ff" />
            </View>
        );
    }

    return (
        <View style={styles.container}>
            {userInfo && (
                <Text style={styles.welcomeText}>
                    Welcome, {userInfo.name}!
                </Text>
            )}
            <View style={styles.inputContainer}>
                <Text style={styles.label}>Enter Class Code:</Text>
                <TextInput
                    style={styles.input}
                    value={classCode}
                    onChangeText={setClassCode}
                    placeholder="Enter your class code"
                    placeholderTextColor="#999"
                />
                <TouchableOpacity
                    style={[
                        styles.button,
                        (!classCode.trim() || isJoining) &&
                            styles.buttonDisabled,
                    ]}
                    onPress={handleEnterClass}
                    disabled={!classCode.trim() || isJoining}
                >
                    {isJoining ? (
                        <ActivityIndicator color="#fff" />
                    ) : (
                        <Text style={styles.buttonText}>Enter Class</Text>
                    )}
                </TouchableOpacity>
            </View>
        </View>
    );
}

const styles = StyleSheet.create({
    container: {
        flex: 1,
        backgroundColor: "#fff",
        padding: 20,
        alignItems: "center",
        justifyContent: "center",
    },
    welcomeText: {
        fontSize: 28,
        fontWeight: "bold",
        color: "#1E3A8A",
        marginBottom: 30,
        textAlign: "center",
    },
    inputContainer: {
        width: "100%",
        maxWidth: 400,
        padding: 20,
    },
    label: {
        fontSize: 18,
        color: "#1E3A8A",
        marginBottom: 10,
    },
    input: {
        borderWidth: 1,
        borderColor: "#1E3A8A",
        borderRadius: 8,
        padding: 15,
        fontSize: 16,
        marginBottom: 20,
        backgroundColor: "#fff",
    },
    button: {
        backgroundColor: "#1E3A8A",
        padding: 15,
        borderRadius: 8,
        alignItems: "center",
    },
    buttonDisabled: {
        backgroundColor: "#CBD5E1",
    },
    buttonText: {
        color: "#fff",
        fontSize: 18,
        fontWeight: "bold",
    },
});
