import SwiftUI

/// Chat message bubble with user vs assistant styling.
/// Uses SCTokens for accent, spacing, and radius.
public struct ChatBubble: View {
    public enum Role {
        case user
        case assistant
    }

    public let text: String
    public let role: Role

    public init(text: String, role: Role) {
        self.text = text
        self.role = role
    }

    public var body: some View {
        HStack(alignment: .bottom, spacing: SCTokens.spaceSm) {
            if role == .user { Spacer(minLength: SCTokens.space2xl) }
            Text(text)
                .padding(.horizontal, 14)
                .padding(.vertical, 10)
                .background(role == .user ? SCTokens.Dark.accent : Color(white: 0.92))
                .foregroundColor(role == .user ? .white : .primary)
                .clipShape(RoundedRectangle(cornerRadius: SCTokens.radiusXl, style: .continuous))
            if role == .assistant { Spacer(minLength: SCTokens.space2xl) }
        }
        .padding(.horizontal, SCTokens.spaceXs)
    }
}

#Preview {
    VStack(spacing: 12) {
        ChatBubble(text: "Hello, how can I help?", role: .assistant)
        ChatBubble(text: "What's the weather today?", role: .user)
    }
    .padding()
}
