import SwiftUI

/// Chat message bubble with user vs assistant styling.
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
        HStack(alignment: .bottom, spacing: 8) {
            if role == .user { Spacer(minLength: 48) }
            Text(text)
                .padding(.horizontal, 14)
                .padding(.vertical, 10)
                .background(role == .user ? Color.accentColor : Color(white: 0.92))
                .foregroundColor(role == .user ? .white : .primary)
                .clipShape(RoundedRectangle(cornerRadius: 16, style: .continuous))
            if role == .assistant { Spacer(minLength: 48) }
        }
        .padding(.horizontal, 4)
    }
}

#Preview {
    VStack(spacing: 12) {
        ChatBubble(text: "Hello, how can I help?", role: .assistant)
        ChatBubble(text: "What's the weather today?", role: .user)
    }
    .padding()
}
