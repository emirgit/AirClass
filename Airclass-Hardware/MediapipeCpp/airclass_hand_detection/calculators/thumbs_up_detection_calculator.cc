#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h" // For NormalizedLandmarkList
#include "mediapipe/framework/port/status.h"
#include <cmath>   // For std::sqrt, std::acos, M_PI
#include <string>
#include <vector>  // For std::vector
#include <algorithm> // For std::min, std::max

// M_PI might not be defined in <cmath> by C++ standard, but often is by POSIX
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace mediapipe {

// ThumbsUpDetectionCalculator detects the thumbs up gesture based on hand landmarks.
// Input:
//   LANDMARKS - std::vector<NormalizedLandmarkList> of hand landmarks (21 points per hand).
// Output:
//   GESTURE - std::string, "THUMBS_UP" if the gesture is detected for the first hand, empty string otherwise.
class ThumbsUpDetectionCalculator : public CalculatorBase {
 public:
  static absl::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag("LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
    cc->Outputs().Tag("GESTURE").Set<std::string>();
    return absl::OkStatus();
  }

  absl::Status Open(CalculatorContext* cc) override {
    cc->SetOffset(TimestampDiff(0)); // Process packets at their original timestamp
    return absl::OkStatus();
  }

  // Calculate the Euclidean distance between two landmarks (2D for simplicity here if z is not critical)
  float DistanceBetweenLandmarks(const NormalizedLandmark& a, const NormalizedLandmark& b) {
    float dx = a.x() - b.x();
    float dy = a.y() - b.y();
    // float dz = a.z() - b.z(); // Can include z for 3D distance
    return std::sqrt(dx * dx + dy * dy /*+ dz * dz*/);
  }

  // Calculate the dot product between two vectors defined by landmarks (origin, p1) and (origin, p2)
  // Or vector (p1-p0) and (p3-p2)
  float DotProduct(const NormalizedLandmark& p0, const NormalizedLandmark& p1,
                   const NormalizedLandmark& p2, const NormalizedLandmark& p3) {
    float v1x = p1.x() - p0.x();
    float v1y = p1.y() - p0.y();
    // float v1z = p1.z() - p0.z();

    float v2x = p3.x() - p2.x();
    float v2y = p3.y() - p2.y();
    // float v2z = p3.z() - p2.z();

    return v1x * v2x + v1y * v2y /*+ v1z * v2z*/;
  }

  // Calculate the magnitude of a vector defined by two landmarks
  float VectorMagnitude(const NormalizedLandmark& p0, const NormalizedLandmark& p1) {
    float dx = p1.x() - p0.x();
    float dy = p1.y() - p0.y();
    // float dz = p1.z() - p0.z();
    return std::sqrt(dx * dx + dy * dy /*+ dz * dz*/);
  }

  // Calculate the angle in degrees between two vectors (p0->p1 and p2->p3)
  float AngleBetweenVectors(const NormalizedLandmark& p0, const NormalizedLandmark& p1,
                            const NormalizedLandmark& p2, const NormalizedLandmark& p3) {
    float dot = DotProduct(p0, p1, p2, p3);
    float mag_v1 = VectorMagnitude(p0, p1);
    float mag_v2 = VectorMagnitude(p2, p3);

    if (mag_v1 < 1e-6 || mag_v2 < 1e-6) return 0.0f; // Avoid division by zero

    float cos_angle = dot / (mag_v1 * mag_v2);
    cos_angle = std::min(1.0f, std::max(-1.0f, cos_angle)); // Clamp to [-1, 1]
    return std::acos(cos_angle) * 180.0f / M_PI;
  }


  bool IsThumbsUp(const NormalizedLandmarkList& landmarks) {
    if (landmarks.landmark_size() < 21) return false; // Need all 21 landmarks

    // Landmark indices (refer to MediaPipe hand landmarks diagram)
    const auto& wrist = landmarks.landmark(0);
    const auto& thumb_cmc = landmarks.landmark(1);
    const auto& thumb_mcp = landmarks.landmark(2);
    const auto& thumb_ip = landmarks.landmark(3);
    const auto& thumb_tip = landmarks.landmark(4);

    const auto& index_mcp = landmarks.landmark(5);
    const auto& index_pip = landmarks.landmark(6);
    const auto& index_dip = landmarks.landmark(7);
    const auto& index_tip = landmarks.landmark(8);

    const auto& middle_mcp = landmarks.landmark(9);
    const auto& middle_pip = landmarks.landmark(10);
    const auto& middle_tip = landmarks.landmark(12);

    const auto& ring_mcp = landmarks.landmark(13);
    const auto& ring_pip = landmarks.landmark(14);
    const auto& ring_tip = landmarks.landmark(16);

    const auto& pinky_mcp = landmarks.landmark(17);
    const auto& pinky_pip = landmarks.landmark(18);
    const auto& pinky_tip = landmarks.landmark(20);

    // Heuristic for Thumbs Up:
    // 1. Thumb is extended: Thumb tip is "above" thumb MCP (y-coordinate is smaller in image space)
    //    and significantly further from wrist than other finger tips.
    bool thumb_extended_up = thumb_tip.y() < thumb_mcp.y() && thumb_mcp.y() < thumb_cmc.y();

    // 2. Other four fingers are curled: Their tips are "below" their PIP or MCP joints.
    bool index_curled = index_tip.y() > index_pip.y() && index_pip.y() > index_mcp.y() * 0.95; // Tip below PIP
    bool middle_curled = middle_tip.y() > middle_pip.y() && middle_pip.y() > middle_mcp.y() * 0.95;
    bool ring_curled = ring_tip.y() > ring_pip.y() && ring_pip.y() > ring_mcp.y() * 0.95;
    bool pinky_curled = pinky_tip.y() > pinky_pip.y() && pinky_pip.y() > pinky_mcp.y() * 0.95;
    
    // Ensure fingers are actually closed towards palm
    // (distance from tip to MCP should be less than MCP to wrist for a curled finger)
    float index_tip_to_mcp = DistanceBetweenLandmarks(index_tip, index_mcp);
    float index_mcp_to_pip = DistanceBetweenLandmarks(index_mcp, index_pip);
    if (index_tip_to_mcp > index_mcp_to_pip * 1.5) index_curled = false; // If tip is too far from MCP

    float middle_tip_to_mcp = DistanceBetweenLandmarks(middle_tip, middle_mcp);
    float middle_mcp_to_pip = DistanceBetweenLandmarks(middle_mcp, middle_pip);
    if (middle_tip_to_mcp > middle_mcp_to_pip* 1.5) middle_curled = false;
    
    // 3. Thumb tip should be significantly higher than index finger MCP
    bool thumb_is_highest = thumb_tip.y() < index_mcp.y() && thumb_tip.y() < middle_mcp.y();

    // 4. Angle check: Angle between vector (Thumb_CMC -> Thumb_TIP) and vector (Index_MCP -> Wrist)
    //    For thumbs up, this angle should be relatively large (e.g., > 70 degrees).
    //    This helps distinguish from a fist or pointing.
    //    Let's define a "palm direction" vector from middle_mcp to wrist.
    //    And a "thumb direction" vector from thumb_cmc to thumb_tip.
    float angle_thumb_palm = AngleBetweenVectors(thumb_cmc, thumb_tip, middle_mcp, wrist);
    bool good_thumb_angle = angle_thumb_palm > 60.0f && angle_thumb_palm < 130.0f;


    // More robust: check angles of finger joints
    // Angle at PIP joint of index finger (MCP-PIP-TIP) should be small for curled finger
    float angle_index_pip = AngleBetweenVectors(index_mcp, index_pip, index_pip, index_tip);
    if (angle_index_pip > 90.0f) index_curled = false; // If finger is somewhat straight

    float angle_middle_pip = AngleBetweenVectors(middle_mcp, middle_pip, middle_pip, middle_tip);
    if (angle_middle_pip > 90.0f) middle_curled = false;

    // Optional: Check if thumb is to the side of the palm (e.g. for right hand, thumb_tip.x > index_mcp.x)
    // This depends on handedness or requires checking both possibilities.
    // For simplicity, we'll rely on the y-coordinates and angles more.

    return thumb_extended_up && index_curled && middle_curled && ring_curled && pinky_curled && thumb_is_highest && good_thumb_angle;
  }

  absl::Status Process(CalculatorContext* cc) override {
    std::string gesture_str = "";

    if (cc->Inputs().Tag("LANDMARKS").IsEmpty()) {
      cc->Outputs().Tag("GESTURE").Add(new std::string(gesture_str), cc->InputTimestamp());
      return absl::OkStatus();
    }

    const auto& landmark_lists = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();

    if (!landmark_lists.empty()) {
      // Process the first detected hand
      const auto& hand_landmarks = landmark_lists[0];
      if (IsThumbsUp(hand_landmarks)) {
        gesture_str = "THUMBS_UP";
      }
    }

    cc->Outputs().Tag("GESTURE").Add(new std::string(gesture_str), cc->InputTimestamp());
    return absl::OkStatus();
  }
};

REGISTER_CALCULATOR(ThumbsUpDetectionCalculator);

}  // namespace mediapipe