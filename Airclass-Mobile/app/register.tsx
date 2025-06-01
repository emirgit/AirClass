import React, { useState } from "react";
import {
    View,
    Text,
    TextInput,
    TouchableOpacity,
    StyleSheet,
    Alert,
} from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useRouter } from "expo-router";

export default function RegisterScreen() {
    const [email, setEmail] = useState("");
    const [name, setName] = useState("");
    const [password, setPassword] = useState("");
    const router = useRouter();

    const handleRegister = async () => {
        if (!email || !name || !password) {
            Alert.alert("Error", "Please fill in all fields");
            return;
        }
        if (!email.toLowerCase().endsWith("@gtu.edu.tr")) {
            Alert.alert(
                "Invalid Email",
                "Please use your GTU email address (@gtu.edu.tr)"
            );
            return;
        }
        try {
            const response = await fetch(
                "http://159.89.19.111/airclass-api/auth/register",
                {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        email,
                        name,
                        password,
                        role: "student",
                    }),
                }
            )
                .then((res) => res.json())
                .then(async (data) => {
                    console.log("Register response data:", data);
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
                                "Registration successful, navigating to login..."
                            );
                            Alert.alert(
                                "Registration Successful",
                                "You can now log in!"
                            );
                            router.replace("/login");
                        } catch (storageError) {
                            console.error("Error storing data:", storageError);
                            Alert.alert(
                                "Error",
                                "Failed to save registration data"
                            );
                        }
                    } else {
                        console.log(
                            "Registration failed - no token in response"
                        );
                        Alert.alert(
                            "Registration Failed",
                            data.message || "Registration failed"
                        );
                    }
                })
                .catch((err) => {
                    console.log(err);
                });
        } catch (err) {
            Alert.alert("Error", "Network or server error");
        }
    };

    return (
        <View style={styles.container}>
            <Text style={styles.title}>Register for AirClass</Text>
            <TextInput
                style={styles.input}
                placeholder="Name"
                placeholderTextColor="#666"
                value={name}
                onChangeText={setName}
            />
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
                autoCapitalize="none"
            />
            <TouchableOpacity style={styles.button} onPress={handleRegister}>
                <Text style={styles.buttonText}>Register</Text>
            </TouchableOpacity>
            <TouchableOpacity onPress={() => router.replace("/login")}>
                <Text style={styles.link}>Already have an account? Log in</Text>
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
