import React, { useState } from "react";
import {
    View,
    TextInput,
    TouchableOpacity,
    Text,
    StyleSheet,
    Image,
} from "react-native";
import { useRouter } from "expo-router";

const REDIRECT_URI = 'http://localhost:8081/(tabs)';

export default function HomeScreen() {
    const [classroomCode, setClassroomCode] = useState("");
    const router = useRouter();

    const handleJoinClassroom = () => {
        if (!classroomCode.trim()) {
            return;
        }
        // Navigate to the classroom screen with the code
        router.push({
            pathname: "/classroom",
            params: { code: classroomCode.trim() },
        });
        router.replace('/(tabs)');
    };

    return (
        <View style={styles.container}>
            <View style={styles.logoContainer}>
                <Image
                    source={require("@/assets/images/icon.png")}
                    style={styles.logo}
                    resizeMode="contain"
                />
                <Text style={styles.title}>AirClass</Text>
                <Text style={styles.subtitle}>Join your virtual classroom</Text>
            </View>

            <View style={styles.inputContainer}>
                <Text style={styles.label}>Enter Classroom Code</Text>
                <TextInput
                    style={styles.input}
                    placeholder="e.g., ABC123"
                    value={classroomCode}
                    onChangeText={setClassroomCode}
                    autoCapitalize="characters"
                    maxLength={6}
                    placeholderTextColor="#666"
                />
                <TouchableOpacity
                    style={[
                        styles.button,
                        !classroomCode.trim() && styles.buttonDisabled,
                    ]}
                    onPress={handleJoinClassroom}
                    disabled={!classroomCode.trim()}
                >
                    <Text style={styles.buttonText}>Join Classroom</Text>
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
    },
    logoContainer: {
        alignItems: "center",
        marginTop: 60,
        marginBottom: 40,
    },
    logo: {
        width: 120,
        height: 120,
        marginBottom: 16,
    },
    title: {
        fontSize: 32,
        fontWeight: "bold",
        color: "#1E3A8A",
        marginBottom: 8,
    },
    subtitle: {
        fontSize: 16,
        color: "#64748B",
        textAlign: "center",
    },
    inputContainer: {
        width: "100%",
        maxWidth: 400,
        alignSelf: "center",
    },
    label: {
        fontSize: 16,
        fontWeight: "600",
        color: "#1E3A8A",
        marginBottom: 8,
    },
    input: {
        height: 50,
        borderWidth: 2,
        borderColor: "#E2E8F0",
        borderRadius: 12,
        paddingHorizontal: 16,
        fontSize: 18,
        color: "#1E3A8A",
        backgroundColor: "#F8FAFC",
        marginBottom: 16,
    },
    button: {
        height: 50,
        backgroundColor: "#2563EB",
        borderRadius: 12,
        justifyContent: "center",
        alignItems: "center",
        shadowColor: "#000",
        shadowOffset: {
            width: 0,
            height: 2,
        },
        shadowOpacity: 0.1,
        shadowRadius: 4,
        elevation: 3,
    },
    buttonDisabled: {
        backgroundColor: "#CBD5E1",
    },
    buttonText: {
        color: "#fff",
        fontSize: 16,
        fontWeight: "600",
    },
});
