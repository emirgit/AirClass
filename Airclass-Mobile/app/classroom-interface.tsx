import React, { useState, useEffect } from "react";
import {
    View,
    Text,
    StyleSheet,
    ActivityIndicator,
    TouchableOpacity,
    Platform,
    Dimensions,
    Alert,
    AppState,
    SafeAreaView,
    TextInput,
    FlatList,
    KeyboardAvoidingView,
    Image,
    Linking,
} from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { useLocalSearchParams, useRouter } from "expo-router";
import { Ionicons } from "@expo/vector-icons";
import * as ScreenOrientation from "expo-screen-orientation";
import { RequestToSpeak } from "../components/RequestToSpeak";
import { WebView, WebViewMessageEvent } from "react-native-webview";

// Theme type definition
type Theme = {
    primary: string;
    primaryLight: string;
    secondary: string;
    background: string;
    surface: string;
    text: string;
    textLight: string;
    border: string;
    success: string;
    error: string;
    warning: string;
    tabBarBg: string;
    selectedBg: string;
    white: string;
    slideIndicatorText: string;
    inputBg: string;
    avatarBg: string;
    avatarText: string;
};

// Theme colors
const LIGHT_THEME: Theme = {
    primary: "#4F46E5", // Indigo
    primaryLight: "#818CF8",
    secondary: "#7C3AED", // Purple
    background: "#F8FAFC",
    surface: "#FFFFFF",
    text: "#1E293B",
    textLight: "#64748B",
    border: "#E2E8F0",
    success: "#10B981",
    error: "#EF4444",
    warning: "#F59E0B",
    tabBarBg: "#FFFFFF",
    selectedBg: "#EEF2FF",
    white: "#FFFFFF",
    slideIndicatorText: "#1E293B",
    inputBg: "#F1F5F9",
    avatarBg: "#4F46E5",
    avatarText: "#FFFFFF",
};

const DARK_THEME: Theme = {
    primary: "#818CF8", // Lighter Indigo
    primaryLight: "#A5B4FC",
    secondary: "#A78BFA", // Lighter Purple
    background: "#0F172A",
    surface: "#1E293B",
    text: "#F8FAFC",
    textLight: "#CBD5E1",
    border: "#334155",
    success: "#34D399",
    error: "#F87171",
    warning: "#FBBF24",
    tabBarBg: "#1E293B",
    selectedBg: "#1E3A8A",
    white: "#FFFFFF",
    slideIndicatorText: "#000000",
    inputBg: "#334155",
    avatarBg: "#818CF8",
    avatarText: "#F8FAFC",
};

const slidesData = [
    { title: "Welcome", content: "Welcome to the class!" },
    { title: "Agenda", content: "Today's agenda: ..." },
    { title: "Topic 1", content: "Let's discuss topic 1." },
    { title: "Q&A", content: "Any questions?" },
];

// Add interfaces for our data types
interface Participant {
    id: number;
    student_name: string;
    student_email: string;
    user_id: number;
    attendance_session_id: number;
    classroom_code: string;
    classroom_id: number;
    teacher_id: number;
    teacher_name: string;
    attendance_date: string;
    attendance_code: string;
    created_at: string;
    updated_at: string;
}

interface AttendanceResponse {
    status: boolean;
    message: string;
    data: {
        total_students: number;
        attendance_list: Participant[];
    };
}

// Add Slide interface
interface Slide {
    id: number;
    classroom_id: number;
    full_path: string;
    created_at: string;
    file_type?: string; // 'pdf' or 'ppt' or 'pptx'
    total_pages?: number;
    current_page?: number;
}

// Add base URL constant
const API_BASE_URL = "http://159.89.19.111/airclass-api";

const createStyles = (COLORS: Theme) =>
    StyleSheet.create({
        container: {
            flex: 1,
            backgroundColor: COLORS.background,
        },
        containerMobile: {
            padding: 16,
        },
        containerExpanded: {
            backgroundColor: COLORS.background,
        },
        safeArea: {
            flex: 1,
            backgroundColor: COLORS.background,
        },
        header: {
            flexDirection: "row",
            justifyContent: "space-between",
            alignItems: "center",
            padding: 16,
            backgroundColor: COLORS.surface,
            borderBottomWidth: 1,
            borderBottomColor: COLORS.border,
        },
        headerContent: {
            flex: 1,
        },
        classroomCode: {
            fontSize: 16,
            fontWeight: "600",
            color: COLORS.text,
        },
        welcomeText: {
            fontSize: 14,
            color: COLORS.textLight,
            marginTop: 4,
        },
        headerButtons: {
            flexDirection: "row",
            alignItems: "center",
            gap: 12,
        },
        themeButton: {
            backgroundColor: COLORS.inputBg,
            padding: 8,
            borderRadius: 12,
            shadowColor: COLORS.primary,
            shadowOffset: { width: 0, height: 2 },
            shadowOpacity: 0.1,
            shadowRadius: 4,
            elevation: 2,
        },
        expandButton: {
            backgroundColor: COLORS.primary,
            padding: 8,
            borderRadius: 12,
            shadowColor: COLORS.primary,
            shadowOffset: { width: 0, height: 2 },
            shadowOpacity: 0.1,
            shadowRadius: 4,
            elevation: 2,
        },
        mainContent: {
            flex: 1,
            flexDirection: "row",
            padding: 24,
            position: "relative",
        },
        mainContentMobile: {
            padding: 16,
            flexDirection: "column",
        },
        mainContentExpanded: {
            padding: 0,
        },
        mainContentLandscape: {
            flexDirection: "row",
            padding: 16,
        },
        slideSection: {
            flex: 4,
            marginRight: 24,
            width: "100%",
        },
        slideSectionMobile: {
            marginRight: 0,
            marginBottom: 24,
        },
        slideSectionExpanded: {
            flex: 1,
            marginRight: 0,
            padding: 16,
        },
        slideSectionLandscape: {
            flex: 3,
            marginRight: 16,
        },
        slideBox: {
            backgroundColor: COLORS.surface,
            borderRadius: 20,
            padding: 24,
            shadowColor: COLORS.primary,
            shadowOffset: { width: 0, height: 4 },
            shadowOpacity: 0.1,
            shadowRadius: 12,
            elevation: 8,
            width: "100%",
            maxWidth: 800,
            alignSelf: "center",
            minHeight: 500,
            flex: 1,
        },
        slideBoxExpanded: {
            flex: 1,
            justifyContent: "center",
            alignItems: "center",
            padding: Math.max(16, Dimensions.get("window").width * 0.02),
            maxWidth: "100%",
            minHeight: "80%",
        },
        slideBoxLandscape: {
            minHeight: 400,
            padding: 20,
            flex: 1,
        },
        expandedHeader: {
            flexDirection: "row",
            justifyContent: "flex-end",
            alignItems: "center",
            marginBottom: 20,
            paddingBottom: 16,
            borderBottomWidth: 1,
            borderBottomColor: COLORS.border,
        },
        slideTitle: {
            fontSize: 24,
            fontWeight: "700",
            color: COLORS.text,
            marginBottom: 16,
            textAlign: "center",
        },
        slideTitleExpanded: {
            fontSize: Math.max(24, Dimensions.get("window").width * 0.03),
            marginBottom: Math.max(16, Dimensions.get("window").width * 0.02),
            textAlign: "center",
        },
        slideTitleLandscape: {
            fontSize: 22,
            marginBottom: 12,
        },
        slideContent: {
            fontSize: 16,
            color: COLORS.textLight,
            lineHeight: 24,
            textAlign: "center",
        },
        slideContentExpanded: {
            fontSize: Math.max(16, Dimensions.get("window").width * 0.02),
            lineHeight: Math.max(24, Dimensions.get("window").width * 0.03),
            textAlign: "center",
            maxWidth: 800,
        },
        slideContentLandscape: {
            fontSize: 16,
            lineHeight: 24,
        },
        slideNavContainer: {
            flexDirection: "row",
            alignItems: "center",
            justifyContent: "space-between",
            width: "100%",
            paddingHorizontal: 16,
            marginTop: 20,
        },
        slideNavContainerExpanded: {
            paddingHorizontal: Math.max(
                16,
                Dimensions.get("window").width * 0.02
            ),
            justifyContent: "center",
            alignItems: "center",
            marginTop: Math.max(48, Dimensions.get("window").width * 0.06),
            position: "relative",
            bottom: -40,
        },
        slideNavContainerLandscape: {
            marginTop: 16,
            paddingHorizontal: 8,
        },
        navButton: {
            backgroundColor: COLORS.primary,
            padding: Math.max(6, Dimensions.get("window").width * 0.01),
            borderRadius: 8,
            opacity: 0.9,
            width: Math.max(32, Dimensions.get("window").width * 0.04),
            height: Math.max(32, Dimensions.get("window").width * 0.04),
            alignItems: "center",
            justifyContent: "center",
            marginHorizontal: Math.max(
                8,
                Dimensions.get("window").width * 0.01
            ),
        },
        navButtonDisabled: {
            backgroundColor: COLORS.border,
            opacity: 0.5,
        },
        slideIndicator: {
            backgroundColor: COLORS.white,
            paddingHorizontal: Math.max(
                10,
                Dimensions.get("window").width * 0.015
            ),
            paddingVertical: Math.max(
                4,
                Dimensions.get("window").width * 0.005
            ),
            borderRadius: 12,
            borderWidth: 1,
            borderColor: COLORS.border,
            minWidth: Math.max(60, Dimensions.get("window").width * 0.08),
            alignItems: "center",
        },
        slideIndicatorText: {
            fontSize: Math.max(12, Dimensions.get("window").width * 0.015),
            fontWeight: "600",
            color: COLORS.slideIndicatorText,
            textAlign: "center",
        },
        sideSection: {
            flex: 1,
            backgroundColor: "transparent",
            padding: 0,
            alignItems: "center",
            justifyContent: "center",
            maxWidth: 400,
            alignSelf: "center",
        },
        sideSectionMobile: {
            marginTop: 16,
            width: "100%",
        },
        sideSectionLandscape: {
            flex: 1,
            padding: 16,
            maxWidth: "100%",
        },
        tabBar: {
            flexDirection: "row",
            backgroundColor: COLORS.tabBarBg,
            borderTopWidth: 1,
            borderTopColor: COLORS.border,
            paddingBottom: Platform.OS === "ios" ? 20 : 8,
            paddingTop: 8,
        },
        tabBarMobile: {
            paddingBottom: Platform.OS === "ios" ? 20 : 8,
        },
        tabBarLandscape: {
            paddingBottom: 8,
        },
        tabItem: {
            flex: 1,
            alignItems: "center",
            justifyContent: "center",
            paddingVertical: 8,
        },
        tabLabel: {
            fontSize: 12,
            fontWeight: "600",
            color: COLORS.primary,
            marginTop: 4,
        },
        tabLabelInactive: {
            color: COLORS.textLight,
        },
        tabLabelActive: {
            color: COLORS.primary,
        },
        filesContainer: {
            flex: 1,
            backgroundColor: COLORS.surface,
            padding: 16,
        },
        filesList: {
            flex: 1,
        },
        fileItem: {
            flexDirection: "row",
            alignItems: "center",
            padding: 16,
            borderBottomWidth: 1,
            borderBottomColor: COLORS.border,
            backgroundColor: COLORS.surface,
        },
        fileIcon: {
            marginRight: 12,
        },
        fileInfo: {
            flex: 1,
        },
        fileName: {
            fontSize: 16,
            fontWeight: "500",
            color: COLORS.text,
            marginBottom: 4,
        },
        fileDate: {
            fontSize: 12,
            color: COLORS.textLight,
        },
        fileType: {
            fontSize: 12,
            color: COLORS.primary,
            backgroundColor: COLORS.selectedBg,
            paddingHorizontal: 8,
            paddingVertical: 4,
            borderRadius: 4,
            marginLeft: 8,
        },
        noFilesContainer: {
            flex: 1,
            alignItems: "center",
            justifyContent: "center",
            padding: 24,
        },
        noFilesText: {
            marginTop: 16,
            fontSize: 16,
            color: COLORS.textLight,
            textAlign: "center",
        },
        sectionTitle: {
            fontSize: 18,
            fontWeight: "600",
            color: COLORS.text,
        },
        participantItem: {
            flexDirection: "row",
            alignItems: "center",
            padding: 16,
            borderBottomWidth: 1,
            borderBottomColor: COLORS.border,
        },
        selectedParticipant: {
            backgroundColor: COLORS.selectedBg,
        },
        participantAvatar: {
            width: 44,
            height: 44,
            borderRadius: 22,
            backgroundColor: COLORS.avatarBg,
            alignItems: "center",
            justifyContent: "center",
            marginRight: 12,
            shadowColor: COLORS.primary,
            shadowOffset: { width: 0, height: 2 },
            shadowOpacity: 0.1,
            shadowRadius: 4,
            elevation: 2,
        },
        avatarText: {
            color: COLORS.avatarText,
            fontSize: 18,
            fontWeight: "600",
        },
        participantInfo: {
            flex: 1,
        },
        participantName: {
            fontSize: 16,
            fontWeight: "500",
            color: COLORS.text,
            marginBottom: 2,
        },
        participantEmail: {
            fontSize: 12,
            color: COLORS.textLight,
        },
        pendingRequestOverlay: {
            position: "absolute",
            bottom: Platform.OS === "ios" ? 180 : 168,
            left: "50%",
            transform: [{ translateX: -175 }],
            backgroundColor: "#7C3AED",
            padding: 20,
            zIndex: 1000,
            width: 350,
            borderRadius: 16,
            shadowColor: "#000",
            shadowOffset: { width: 0, height: -2 },
            shadowOpacity: 0.1,
            shadowRadius: 4,
            elevation: 4,
            marginHorizontal: 20,
        },
        pendingRequestContainer: {
            alignItems: "center",
            justifyContent: "center",
            width: "100%",
        },
        pendingRequestText: {
            color: COLORS.white,
            fontSize: 18,
            fontWeight: "600",
            textAlign: "center",
            width: "100%",
            marginBottom: 4,
        },
        pendingRequestSubtext: {
            color: COLORS.white,
            fontSize: 15,
            opacity: 0.9,
            marginTop: 4,
            marginBottom: 16,
            textAlign: "center",
            width: "100%",
            lineHeight: 20,
        },
        pendingRequestButtons: {
            flexDirection: "row",
            gap: 12,
            marginTop: 8,
            width: "100%",
            justifyContent: "center",
        },
        pendingRequestButton: {
            flex: 1,
            backgroundColor: "rgba(255, 255, 255, 0.2)",
            paddingVertical: 10,
            paddingHorizontal: 20,
            borderRadius: 10,
            borderWidth: 1,
            borderColor: "rgba(255, 255, 255, 0.3)",
            alignItems: "center",
            justifyContent: "center",
            maxWidth: 160,
        },
        pendingRequestButtonDanger: {
            backgroundColor: "rgba(239, 68, 68, 0.2)",
            borderColor: "rgba(239, 68, 68, 0.3)",
        },
        pendingRequestButtonText: {
            color: COLORS.white,
            fontSize: 14,
            fontWeight: "500",
        },
        slideImage: {
            width: "100%",
            height: 400,
            borderRadius: 12,
            backgroundColor: COLORS.surface,
        },
        slideImageExpanded: {
            height: "80%",
            maxHeight: 800,
        },
        slideImageLandscape: {
            height: 300,
        },
        noSlidesContainer: {
            flex: 1,
            alignItems: "center",
            justifyContent: "center",
            padding: 24,
        },
        noSlidesText: {
            marginTop: 16,
            fontSize: 16,
            color: COLORS.textLight,
            textAlign: "center",
        },
    });

// Add file type detection function
const getFileType = (path: string): string => {
    const extension = path.split(".").pop()?.toLowerCase();
    if (extension === "pdf") return "pdf";
    if (["ppt", "pptx"].includes(extension || "")) return "ppt";
    return "unknown";
};

// Update the PDFViewer component
const PDFViewer = ({
    uri,
    currentPage,
    onPageChange,
}: {
    uri: string;
    currentPage: number;
    onPageChange: (page: number) => void;
}) => {
    const pdfUrl = `${uri}#page=${currentPage}`;
    console.log("[PDFViewer] Component mounted with:", {
        uri,
        currentPage,
        pdfUrl,
        timestamp: new Date().toISOString(),
    });

    return (
        <View style={{ flex: 1, minHeight: 400, backgroundColor: "#fff" }}>
            <WebView
                source={{
                    uri: pdfUrl,
                    headers: {
                        Accept: "application/pdf",
                        "Content-Type": "application/pdf",
                    },
                }}
                style={{
                    flex: 1,
                    backgroundColor: "#fff",
                }}
                originWhitelist={["*"]}
                onLoadStart={() =>
                    console.log("[PDFViewer] Loading started:", pdfUrl)
                }
                onLoad={() =>
                    console.log("[PDFViewer] Loaded successfully:", pdfUrl)
                }
                onError={(syntheticEvent) => {
                    const { nativeEvent } = syntheticEvent;
                    console.error("[PDFViewer] Loading error:", {
                        description: nativeEvent.description,
                        url: nativeEvent.url,
                        code: nativeEvent.code,
                        timestamp: new Date().toISOString(),
                    });
                }}
                onHttpError={(syntheticEvent) => {
                    const { nativeEvent } = syntheticEvent;
                    console.error("[PDFViewer] HTTP error:", {
                        statusCode: nativeEvent.statusCode,
                        url: nativeEvent.url,
                        timestamp: new Date().toISOString(),
                    });
                }}
                onMessage={(event: WebViewMessageEvent) => {
                    console.log("[PDFViewer] Message received:", {
                        data: event.nativeEvent.data,
                        timestamp: new Date().toISOString(),
                    });
                }}
                javaScriptEnabled={true}
                domStorageEnabled={true}
                startInLoadingState={true}
                scalesPageToFit={true}
                bounces={false}
                scrollEnabled={true}
                renderLoading={() => (
                    <View
                        style={{
                            flex: 1,
                            justifyContent: "center",
                            alignItems: "center",
                            position: "absolute",
                            top: 0,
                            left: 0,
                            right: 0,
                            bottom: 0,
                            backgroundColor: "#fff",
                        }}
                    >
                        <ActivityIndicator size="large" color="#4F46E5" />
                    </View>
                )}
                injectedJavaScript={`
                    (function() {
                        window.ReactNativeWebView.postMessage(JSON.stringify({
                            type: 'pdfLoaded',
                            url: window.location.href
                        }));
                        
                        // Try to force PDF display
                        document.body.style.margin = '0';
                        document.body.style.padding = '0';
                        document.body.style.overflow = 'hidden';
                        
                        // Add viewport meta tag if not present
                        if (!document.querySelector('meta[name="viewport"]')) {
                            const meta = document.createElement('meta');
                            meta.name = 'viewport';
                            meta.content = 'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no';
                            document.head.appendChild(meta);
                        }
                    })();
                `}
            />
        </View>
    );
};

// Update the PowerPointViewer component
const PowerPointViewer = ({
    uri,
    currentPage,
    onPageChange,
}: {
    uri: string;
    currentPage: number;
    onPageChange: (page: number) => void;
}) => {
    const pptUrl = `https://view.officeapps.live.com/op/embed.aspx?src=${encodeURIComponent(
        uri
    )}&wdStartOn=${currentPage}`;
    console.log("[PowerPointViewer] Component mounted with:", {
        uri,
        currentPage,
        pptUrl,
        timestamp: new Date().toISOString(),
    });

    return (
        <WebView
            source={{ uri: pptUrl }}
            style={{ flex: 1, backgroundColor: "#fff", minHeight: 400 }}
            onLoadStart={() =>
                console.log("[PowerPointViewer] Loading started:", pptUrl)
            }
            onLoad={() =>
                console.log("[PowerPointViewer] Loaded successfully:", pptUrl)
            }
            onError={(syntheticEvent) => {
                const { nativeEvent } = syntheticEvent;
                console.error("[PowerPointViewer] Loading error:", {
                    description: nativeEvent.description,
                    url: nativeEvent.url,
                    code: nativeEvent.code,
                    timestamp: new Date().toISOString(),
                });
            }}
            onHttpError={(syntheticEvent) => {
                const { nativeEvent } = syntheticEvent;
                console.error("[PowerPointViewer] HTTP error:", {
                    statusCode: nativeEvent.statusCode,
                    url: nativeEvent.url,
                    timestamp: new Date().toISOString(),
                });
            }}
            onMessage={(event: WebViewMessageEvent) => {
                console.log("[PowerPointViewer] Message received:", {
                    data: event.nativeEvent.data,
                    timestamp: new Date().toISOString(),
                });
            }}
            javaScriptEnabled={true}
            domStorageEnabled={true}
            startInLoadingState={true}
            renderLoading={() => (
                <View
                    style={{
                        flex: 1,
                        justifyContent: "center",
                        alignItems: "center",
                    }}
                >
                    <ActivityIndicator size="large" color="#4F46E5" />
                </View>
            )}
        />
    );
};

export default function ClassroomInterfaceScreen() {
    const { code } = useLocalSearchParams<{ code: string }>();
    const [isRequestAccepted, setIsRequestAccepted] = useState(false);
    const [studentId] = useState("STUDENT123"); // This should come from your auth system
    const [userInfo, setUserInfo] = useState<any>(null);
    const [isLoading, setIsLoading] = useState(true);
    const [slideIndex, setSlideIndex] = useState(0);
    const router = useRouter();
    const [isExpanded, setIsExpanded] = useState(false);
    const [dimensions, setDimensions] = useState(Dimensions.get("window"));
    const [activeTab, setActiveTab] = useState<"class" | "files">("class");
    const [isParticipantsOpen, setIsParticipantsOpen] = useState(true);
    const [selectedParticipant, setSelectedParticipant] =
        useState<Participant | null>(null);
    const [isDarkMode, setIsDarkMode] = useState(false);
    const [requestStatus, setRequestStatus] = useState<any>(null);
    const [slides, setSlides] = useState<Slide[]>([]);
    const [isLoadingSlides, setIsLoadingSlides] = useState(true);
    const [currentPage, setCurrentPage] = useState(1);
    const [totalPages, setTotalPages] = useState(1);
    const [fileType, setFileType] = useState<string>("unknown");
    const COLORS = isDarkMode ? DARK_THEME : LIGHT_THEME;
    const styles = createStyles(COLORS);
    const requestToSpeakRef = React.useRef<any>(null);

    // Handle orientation changes
    useEffect(() => {
        const subscription = Dimensions.addEventListener(
            "change",
            ({ window }) => {
                setDimensions(window);
                if (window.width > window.height) {
                    setIsExpanded(true);
                } else {
                    setIsExpanded(false);
                }
            }
        );

        return () => subscription?.remove();
    }, []);

    const isLandscape = dimensions.width > dimensions.height;
    const isMobile = dimensions.width < 700;

    // Handle orientation changes
    const handleExpand = async () => {
        if (!isExpanded) {
            // Lock to landscape when expanding
            await ScreenOrientation.lockAsync(
                ScreenOrientation.OrientationLock.LANDSCAPE
            );
        } else {
            // Unlock orientation when collapsing
            await ScreenOrientation.unlockAsync();
        }
        setIsExpanded(!isExpanded);
    };

    // Reset orientation when component unmounts
    useEffect(() => {
        return () => {
            ScreenOrientation.unlockAsync();
        };
    }, []);

    // Add leave classroom function
    const leaveClassroom = async () => {
        try {
            const token = await AsyncStorage.getItem("jwtToken");
            if (!token) return;

            // Call the leave endpoint
            const response = await fetch(
                "http://159.89.19.111/airclass-api/classroom/leave",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Authorization: `Bearer ${token}`,
                    },
                }
            );

            const data = await response.json();
            console.log("Leave classroom response:", data);

            // Clear classroom data from storage regardless of API response
            await AsyncStorage.removeItem("code");
            await AsyncStorage.removeItem("classroomId");
        } catch (error) {
            console.error("Error leaving classroom:", error);
            // Still try to clear storage even if API call fails
            try {
                await AsyncStorage.removeItem("code");
                await AsyncStorage.removeItem("classroomId");
            } catch (storageError) {
                console.error("Error clearing classroom data:", storageError);
            }
        }
    };

    // Handle app state changes - only when app is closed
    useEffect(() => {
        const subscription = AppState.addEventListener(
            "change",
            (nextAppState) => {
                if (nextAppState === "background") {
                    // Only leave when app goes to background (closed)
                    leaveClassroom();
                }
            }
        );

        return () => {
            subscription.remove();
        };
    }, []);

    useEffect(() => {
        const getUserInfoFromStorage = async () => {
            try {
                const userEmail = await AsyncStorage.getItem("userEmail");
                const userName = await AsyncStorage.getItem("userName");
                if (userEmail && userName) {
                    setUserInfo({ email: userEmail, name: userName });
                }
            } catch (error) {
                // ignore
            } finally {
                setIsLoading(false);
            }
        };
        getUserInfoFromStorage();
    }, []);

    // Fetch class participants
    useEffect(() => {
        const fetchParticipants = async () => {
            try {
                const token = await AsyncStorage.getItem("jwtToken");
                if (!token) return;

                const response = await fetch(
                    `http://159.89.19.111/airclass-api/attendance?code=${code}`,
                    {
                        method: "GET",
                        headers: {
                            "Content-Type": "application/json",
                            Authorization: `Bearer ${token}`,
                        },
                    }
                );

                const data: AttendanceResponse = await response.json();
                if (data.status && data.data?.attendance_list) {
                    setSelectedParticipant(data.data.attendance_list[0]);
                    console.log(`Fetched ${data.data.total_students} students`);
                } else {
                    console.error(
                        "Failed to fetch participants:",
                        data.message
                    );
                }
            } catch (error) {
                console.error("Error fetching participants:", error);
            }
        };

        if (code) {
            fetchParticipants();
        }
    }, [code]);

    // Update the slide fetching effect to include more detailed logging
    useEffect(() => {
        const fetchSlides = async () => {
            try {
                const token = await AsyncStorage.getItem("jwtToken");
                const classroomId = await AsyncStorage.getItem("classroomId");
                if (!token || !classroomId) {
                    console.error("[fetchSlides] Missing required data:", {
                        hasToken: !!token,
                        classroomId,
                        timestamp: new Date().toISOString(),
                    });
                    return;
                }

                console.log("[fetchSlides] Starting fetch for classroom:", {
                    classroomId,
                    timestamp: new Date().toISOString(),
                });

                const response = await fetch(
                    `${API_BASE_URL}/airclass-api/slide?classroom_id=${classroomId}`,
                    {
                        method: "GET",
                        headers: {
                            Authorization: `Bearer ${token}`,
                        },
                    }
                );

                const data = await response.json();
                console.log("[fetchSlides] API Response:", {
                    status: data.status,
                    message: data.message,
                    slideCount: Array.isArray(data.data) ? data.data.length : 0,
                    timestamp: new Date().toISOString(),
                });

                if (data.status && Array.isArray(data.data)) {
                    const processedSlides = data.data.map((slide: Slide) => {
                        const fileType = getFileType(slide.full_path);
                        const fullUrl = `${API_BASE_URL}${slide.full_path}`;
                        console.log("[fetchSlides] Processing slide:", {
                            id: slide.id,
                            path: slide.full_path,
                            fileType,
                            fullUrl,
                            timestamp: new Date().toISOString(),
                        });
                        return {
                            ...slide,
                            file_type: fileType,
                        };
                    });

                    setSlides(processedSlides);

                    if (processedSlides.length > 0) {
                        const firstSlide = processedSlides[0];
                        console.log("[fetchSlides] Setting initial slide:", {
                            id: firstSlide.id,
                            fileType: firstSlide.file_type,
                            url: `${API_BASE_URL}${firstSlide.full_path}`,
                            timestamp: new Date().toISOString(),
                        });
                        setFileType(firstSlide.file_type || "unknown");
                        setTotalPages(firstSlide.file_type === "pdf" ? 1 : 1);
                    }
                } else {
                    console.error("[fetchSlides] Invalid API response:", {
                        status: data.status,
                        message: data.message,
                        data: data.data,
                        timestamp: new Date().toISOString(),
                    });
                }
            } catch (error) {
                console.error("[fetchSlides] Error:", {
                    error,
                    timestamp: new Date().toISOString(),
                });
            } finally {
                setIsLoadingSlides(false);
            }
        };

        fetchSlides();
    }, []);

    // Update the slide navigation functions
    const goToPrevSlide = () => {
        setSlideIndex((prev) => Math.max(prev - 1, 0));
    };

    const goToNextSlide = () => {
        setSlideIndex((prev) => Math.min(prev + 1, slides.length - 1));
    };

    // Update page navigation functions
    const goToPrevPage = () => {
        if (currentPage > 1) {
            setCurrentPage((prev) => prev - 1);
        }
    };

    const goToNextPage = () => {
        if (currentPage < totalPages) {
            setCurrentPage((prev) => prev + 1);
        }
    };

    const handleAttendance = () => {
        router.push("/attendance");
    };

    const renderSlideSection = () => (
        <View
            style={[
                styles.slideSection,
                isMobile && styles.slideSectionMobile,
                isExpanded && styles.slideSectionExpanded,
                isLandscape && !isExpanded && styles.slideSectionLandscape,
            ]}
        >
            <View
                style={[
                    styles.slideBox,
                    isExpanded && styles.slideBoxExpanded,
                    isLandscape && !isExpanded && styles.slideBoxLandscape,
                ]}
            >
                {isLoadingSlides ? (
                    <ActivityIndicator size="large" color={COLORS.primary} />
                ) : slides.length > 0 ? (
                    <>
                        {fileType === "pdf" ? (
                            <PDFViewer
                                uri={`${API_BASE_URL}${slides[slideIndex]?.full_path}`}
                                currentPage={currentPage}
                                onPageChange={setCurrentPage}
                            />
                        ) : fileType === "ppt" ? (
                            <PowerPointViewer
                                uri={`${API_BASE_URL}${slides[slideIndex]?.full_path}`}
                                currentPage={currentPage}
                                onPageChange={setCurrentPage}
                            />
                        ) : (
                            <View style={styles.noSlidesContainer}>
                                <Ionicons
                                    name="document-outline"
                                    size={48}
                                    color={COLORS.textLight}
                                />
                                <Text style={styles.noSlidesText}>
                                    Unsupported file type
                                </Text>
                            </View>
                        )}

                        {/* Page Navigation */}
                        <View
                            style={[
                                styles.slideNavContainer,
                                isExpanded && styles.slideNavContainerExpanded,
                                isLandscape &&
                                    !isExpanded &&
                                    styles.slideNavContainerLandscape,
                            ]}
                        >
                            <TouchableOpacity
                                style={[
                                    styles.navButton,
                                    currentPage === 1 &&
                                        styles.navButtonDisabled,
                                ]}
                                onPress={goToPrevPage}
                                disabled={currentPage === 1}
                            >
                                <Ionicons
                                    name="chevron-back"
                                    size={20}
                                    color={COLORS.white}
                                />
                            </TouchableOpacity>

                            <View style={styles.slideIndicator}>
                                <Text style={styles.slideIndicatorText}>
                                    Page {currentPage} of {totalPages}
                                </Text>
                            </View>

                            <TouchableOpacity
                                style={[
                                    styles.navButton,
                                    currentPage === totalPages &&
                                        styles.navButtonDisabled,
                                ]}
                                onPress={goToNextPage}
                                disabled={currentPage === totalPages}
                            >
                                <Ionicons
                                    name="chevron-forward"
                                    size={20}
                                    color={COLORS.white}
                                />
                            </TouchableOpacity>
                        </View>
                    </>
                ) : (
                    <View style={styles.noSlidesContainer}>
                        <Ionicons
                            name="document-outline"
                            size={48}
                            color={COLORS.textLight}
                        />
                        <Text style={styles.noSlidesText}>
                            No slides available
                        </Text>
                    </View>
                )}
            </View>
        </View>
    );

    const renderHeader = () => (
        <View style={styles.header}>
            <View style={styles.headerContent}>
                <Text style={styles.classroomCode}>Classroom: {code}</Text>
                <Text style={styles.welcomeText}>
                    Welcome, {userInfo?.name}!
                </Text>
            </View>
            <View style={styles.headerButtons}>
                <TouchableOpacity
                    style={styles.themeButton}
                    onPress={() => setIsDarkMode(!isDarkMode)}
                    activeOpacity={0.7}
                >
                    <Ionicons
                        name={isDarkMode ? "sunny" : "moon"}
                        size={24}
                        color={COLORS.text}
                    />
                </TouchableOpacity>
                <TouchableOpacity
                    style={styles.expandButton}
                    onPress={handleExpand}
                    activeOpacity={0.7}
                >
                    <Ionicons
                        name={isExpanded ? "contract" : "expand"}
                        size={24}
                        color={COLORS.white}
                    />
                </TouchableOpacity>
            </View>
        </View>
    );

    const renderPendingRequestOverlay = (requestStatus: any) => {
        if (!requestStatus || requestStatus.status !== "pending") return null;

        return (
            <View style={styles.pendingRequestOverlay}>
                <View style={styles.pendingRequestContainer}>
                    <Text style={styles.pendingRequestText}>
                        Request Pending
                    </Text>
                    <Text style={styles.pendingRequestSubtext}>
                        Waiting for instructor's approval...
                    </Text>
                    <View style={styles.pendingRequestButtons}>
                        <TouchableOpacity
                            style={styles.pendingRequestButton}
                            onPress={() =>
                                requestToSpeakRef.current?.handleCheckRequest()
                            }
                        >
                            <Text style={styles.pendingRequestButtonText}>
                                Check Request
                            </Text>
                        </TouchableOpacity>
                        <TouchableOpacity
                            style={[
                                styles.pendingRequestButton,
                                styles.pendingRequestButtonDanger,
                            ]}
                            onPress={() =>
                                requestToSpeakRef.current?.handleCancelRequest()
                            }
                        >
                            <Text style={styles.pendingRequestButtonText}>
                                Cancel Request
                            </Text>
                        </TouchableOpacity>
                    </View>
                </View>
            </View>
        );
    };

    // Add FilesSection component after styles and COLORS are defined
    const FilesSection = () => {
        const handleFilePress = (slide: Slide) => {
            const fileUrl = `${API_BASE_URL}${slide.full_path}`;
            Linking.openURL(fileUrl).catch((err) => {
                console.error("Error opening file:", err);
                Alert.alert(
                    "Error",
                    "Could not open the file. Please try again later."
                );
            });
        };

        const getFileIcon = (fileType?: string) => {
            switch (fileType) {
                case "pdf":
                    return "document-text";
                case "ppt":
                case "pptx":
                    return "easel";
                default:
                    return "document";
            }
        };

        const formatDate = (dateString: string) => {
            const date = new Date(dateString);
            return date.toLocaleDateString("en-US", {
                year: "numeric",
                month: "short",
                day: "numeric",
            });
        };

        if (slides.length === 0) {
            return (
                <View style={styles.noFilesContainer}>
                    <Ionicons
                        name="document-outline"
                        size={48}
                        color={COLORS.textLight}
                    />
                    <Text style={styles.noFilesText}>No files available</Text>
                </View>
            );
        }

        return (
            <View style={styles.filesContainer}>
                <Text style={styles.sectionTitle}>Class Files</Text>
                <FlatList
                    data={slides}
                    keyExtractor={(item) => item.id.toString()}
                    style={styles.filesList}
                    renderItem={({ item }) => (
                        <TouchableOpacity
                            style={styles.fileItem}
                            onPress={() => handleFilePress(item)}
                            activeOpacity={0.7}
                        >
                            <Ionicons
                                name={getFileIcon(item.file_type)}
                                size={24}
                                color={COLORS.primary}
                                style={styles.fileIcon}
                            />
                            <View style={styles.fileInfo}>
                                <Text style={styles.fileName}>
                                    {item.full_path.split("/").pop()}
                                </Text>
                                <Text style={styles.fileDate}>
                                    Added on {formatDate(item.created_at)}
                                </Text>
                            </View>
                            <Text style={styles.fileType}>
                                {item.file_type?.toUpperCase() || "UNKNOWN"}
                            </Text>
                        </TouchableOpacity>
                    )}
                />
            </View>
        );
    };

    if (isLoading) {
        return (
            <View style={styles.container}>
                <ActivityIndicator size="large" color="#0000ff" />
            </View>
        );
    }

    return (
        <SafeAreaView style={styles.safeArea}>
            <View
                style={[
                    styles.container,
                    isMobile && styles.containerMobile,
                    isExpanded && styles.containerExpanded,
                ]}
            >
                {renderHeader()}
                {renderPendingRequestOverlay(requestStatus)}

                {/* Main Content Section */}
                <View
                    style={[
                        styles.mainContent,
                        isMobile && styles.mainContentMobile,
                        isExpanded && styles.mainContentExpanded,
                        isLandscape &&
                            !isExpanded &&
                            styles.mainContentLandscape,
                    ]}
                >
                    {activeTab === "class" && (
                        <>
                            {renderSlideSection()}
                            {/* Side Section */}
                            {!isExpanded && (
                                <View
                                    style={[
                                        styles.sideSection,
                                        isMobile && styles.sideSectionMobile,
                                        isLandscape &&
                                            styles.sideSectionLandscape,
                                    ]}
                                >
                                    <RequestToSpeak
                                        studentId={studentId}
                                        onRequestAccepted={() =>
                                            setIsRequestAccepted(true)
                                        }
                                    />
                                </View>
                            )}
                        </>
                    )}
                    {activeTab === "files" && <FilesSection />}
                </View>

                {/* Bottom Navigation Bar */}
                {!isExpanded && (
                    <View
                        style={[
                            styles.tabBar,
                            isMobile && styles.tabBarMobile,
                            isLandscape && styles.tabBarLandscape,
                        ]}
                    >
                        <TouchableOpacity
                            style={styles.tabItem}
                            activeOpacity={0.7}
                            onPress={() => setActiveTab("class")}
                        >
                            <Ionicons
                                name="people"
                                size={24}
                                color={
                                    activeTab === "class"
                                        ? COLORS.primary
                                        : COLORS.textLight
                                }
                            />
                            <Text
                                style={[
                                    styles.tabLabel,
                                    activeTab === "class"
                                        ? styles.tabLabelActive
                                        : styles.tabLabelInactive,
                                ]}
                            >
                                Class
                            </Text>
                        </TouchableOpacity>

                        <TouchableOpacity
                            style={styles.tabItem}
                            activeOpacity={0.7}
                            onPress={() => setActiveTab("files")}
                        >
                            <Ionicons
                                name="document-text"
                                size={24}
                                color={
                                    activeTab === "files"
                                        ? COLORS.primary
                                        : COLORS.textLight
                                }
                            />
                            <Text
                                style={[
                                    styles.tabLabel,
                                    activeTab === "files"
                                        ? styles.tabLabelActive
                                        : styles.tabLabelInactive,
                                ]}
                            >
                                Files
                            </Text>
                        </TouchableOpacity>
                    </View>
                )}
            </View>
        </SafeAreaView>
    );
}
