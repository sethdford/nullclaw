import SwiftUI

/// Text field and send button for chat input.
public struct ChatInputBar: View {
    @Binding public var text: String
    public let onSend: () -> Void
    public var placeholder: String = "Message"

    public init(text: Binding<String>, onSend: @escaping () -> Void, placeholder: String = "Message") {
        self._text = text
        self.onSend = onSend
        self.placeholder = placeholder
    }

    public var body: some View {
        HStack(spacing: SCTokens.spaceMd) {
            TextField(placeholder, text: $text, axis: .vertical)
                .textFieldStyle(.plain)
                .lineLimit(1...6)
                .padding(.horizontal, 14)
                .padding(.vertical, 10)
                .background(Color(white: 0.95))
                .clipShape(RoundedRectangle(cornerRadius: 20, style: .continuous))

            Button(action: onSend) {
                Image(systemName: "arrow.up.circle.fill")
                    .font(.title2)
                    .foregroundStyle(text.isEmpty ? Color.secondary : SCTokens.Dark.accent)
            }
            .disabled(text.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
        }
        .padding(.horizontal, SCTokens.spaceMd)
        .padding(.vertical, SCTokens.spaceSm)
    }
}

#Preview {
    struct PreviewWrapper: View {
        @State private var text = ""
        var body: some View {
            ChatInputBar(text: $text) {}
        }
    }
    return PreviewWrapper()
        .padding()
}
