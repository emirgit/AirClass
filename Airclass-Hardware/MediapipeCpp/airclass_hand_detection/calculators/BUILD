# mediapipe/examples/desktop/airclass_hand_detection/calculators/BUILD

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "thumbs_up_detection_calculator",
    srcs = ["thumbs_up_detection_calculator.cc"],
    hdrs = [], # No separate headers for this simple calculator
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:landmark_cc_proto", # For NormalizedLandmarkList
        "//mediapipe/framework/port:status",
        "@com_google_absl//absl/strings", # For std::string
    ],
    alwayslink = 1, # IMPORTANT: Ensures the calculator is registered
)