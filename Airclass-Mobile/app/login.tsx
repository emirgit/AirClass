import React, { useState } from "react";
import {
    View,
    Text,
    TextInput,
    TouchableOpacity,
    StyleSheet,
    Alert,
    ActivityIndicator,
} from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useRouter } from "expo-router";

export default function LoginScreen() {
    const [email, setEmail] = useState("");
    const [password, setPassword] = useState("");
    const [isLoading, setIsLoading] = useState(false);
    const router = useRouter();

    const handleLogin = async () => {
        setIsLoading(true);
        if (!email || !password) {
            Alert.alert("Error", "Please enter both email and password");
            setIsLoading(false);
            return;
        }
        try {
            const response = await fetch(
                "http://159.89.19.111/airclass-api/auth/login",
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ email, password }),
                }
            )
                .then((res) => res.json())
                .then(async (data) => {
                    console.log("Login response data:", data);
                    if (data.data && data.data.token) {
                        try {
                            console.log("Storing token and user data...");
                            await AsyncStorage.setItem(
                                "jwtToken",
                                data.data.token
                            );
                            await AsyncStorage.setItem(
                                "userEmail",
                                data.data.user.email
                            );
                            await AsyncStorage.setItem(
                                "userName",
                                data.data.user.name
                            );
                            console.log(
                                "Data stored successfully, navigating to classroom..."
                            );
                            Alert.alert(
                                "Login Successful",
                                `Welcome ${data.data.user.name}!`
                            );
                            // Use setTimeout to ensure Alert is shown before navigation
                            setTimeout(() => {
                                console.log(
                                    "Attempting navigation to classroom..."
                                );
                                router.replace("/classroom");
                            }, 100);
                        } catch (storageError) {
                            console.error("Error storing data:", storageError);
                            Alert.alert("Error", "Failed to save login data");
                        }
                    } else {
                        console.log("Login failed - no token in response");
                        Alert.alert(
                            "Login Failed",
                            data.message || "Invalid credentials"
                        );
                    }
                })
                .catch((err) => {
                    console.log(err);
                    Alert.alert("Error", "Network or server error");
                })
                .finally(() => {
                    setIsLoading(false);
                });
        } catch (err) {
            Alert.alert("Error", "Network or server error");
            setIsLoading(false);
        }
    };

    return (
        <View style={styles.container}>
            <Text style={styles.title}>Sign In to AirClass</Text>
            <TextInput
                style={styles.input}
                placeholder="Email"
                placeholderTextColor="#666"
                value={email}
                onChangeText={setEmail}
                autoCapitalize="none"
                keyboardType="email-address"
            />
            <TextInput
                style={styles.input}
                placeholder="Password"
                placeholderTextColor="#666"
                value={password}
                onChangeText={setPassword}
                secureTextEntry
            />
            <TouchableOpacity
                style={styles.button}
                onPress={handleLogin}
                disabled={isLoading}
            >
                {isLoading ? (
                    <ActivityIndicator color="#fff" />
                ) : (
                    <Text style={styles.buttonText}>Sign In</Text>
                )}
            </TouchableOpacity>
            <TouchableOpacity onPress={() => router.replace("/register")}>
                <Text style={styles.link}>Don't have an account? Register</Text>
            </TouchableOpacity>
        </View>
    );
}

const styles = StyleSheet.create({
    container: {
        flex: 1,
        backgroundColor: "#fff",
        justifyContent: "center",
        alignItems: "center",
        padding: 20,
    },
    title: { fontSize: 24, fontWeight: "bold", marginBottom: 30 },
    input: {
        width: "100%",
        height: 50,
        borderWidth: 1,
        borderColor: "#ddd",
        borderRadius: 8,
        paddingHorizontal: 15,
        marginBottom: 20,
        fontSize: 16,
        color: "#000",
    },
    button: {
        width: "100%",
        height: 50,
        backgroundColor: "#4F46E5",
        borderRadius: 8,
        justifyContent: "center",
        alignItems: "center",
    },
    buttonText: { color: "#fff", fontSize: 16, fontWeight: "600" },
    link: { color: "#4F46E5", marginTop: 20 },
});
