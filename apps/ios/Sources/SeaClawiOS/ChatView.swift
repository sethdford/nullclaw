import SwiftUI
import SeaClawChatUI
import SeaClawProtocol

struct ChatView: View {
    @EnvironmentObject var connectionManager: ConnectionManager
    @State private var messages: [ChatMessage] = []
    @State private var inputText = ""
    @State private var toolCalls: [ToolCallInfo] = []
    @State private var errorBanner: String?

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                ScrollViewReader { proxy in
                    ScrollView {
                        LazyVStack(alignment: .leading, spacing: SCTokens.spaceMd) {
                            ForEach(messages) { msg in
                                ChatBubble(text: msg.text, role: msg.role)
                                    .id(msg.id)
                            }
                            ForEach(toolCalls) { tc in
                                ToolCallCard(
                                    name: tc.name,
                                    arguments: tc.arguments,
                                    status: tc.status,
                                    result: tc.result
                                )
                                .id(tc.id)
                            }
                        }
                        .padding()
                    }
                    .onChange(of: messages.count) { _, _ in
                        if let last = messages.last {
                            withAnimation(SCTokens.springExpressive) { proxy.scrollTo(last.id, anchor: .bottom) }
                        }
                    }
                }

                if let err = errorBanner {
                    HStack {
                        Text(err)
                            .font(.caption)
                            .foregroundStyle(SCTokens.Dark.error)
                        Spacer()
                        Button("Dismiss") { errorBanner = nil }
                            .font(.caption)
                    }
                    .padding(SCTokens.spaceSm)
                    .background(SCTokens.Dark.errorDim)
                    .cornerRadius(SCTokens.radiusMd)
                }
                ChatInputBar(text: $inputText) {
                    sendMessage()
                }
                .background(Color(white: 1))
            }
            .navigationTitle("Chat")
            .toolbar {
                ToolbarItem(placement: .primaryAction) {
                        HStack(spacing: SCTokens.spaceSm) {
                        Circle()
                            .fill(connectionManager.isConnected ? SCTokens.Dark.success : SCTokens.Dark.error)
                            .frame(width: 8, height: 8)
                        if connectionManager.isConnected {
                            Text("Connected")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        }
                    }
                }
            }
            .onAppear {
                connectionManager.setEventHandler { event, payload in
                    handleEvent(event, payload: payload)
                }
                connectionManager.connect()
            }
        }
    }

    private func sendMessage() {
        let trimmed = inputText.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return }
        inputText = ""

        messages.append(ChatMessage(id: UUID(), text: trimmed, role: .user))

        Task {
            do {
                _ = try await connectionManager.request(
                    method: Methods.chatSend,
                    params: ["message": AnyCodable(trimmed)]
                )
            } catch {
                await MainActor.run {
                    messages.append(ChatMessage(
                        id: UUID(),
                        text: "Failed to send: \(error.localizedDescription)",
                        role: .assistant
                    ))
                }
            }
        }
    }

    private func handleEvent(_ event: String, payload: [String: AnyCodable]?) {
        DispatchQueue.main.async {
            switch event {
            case "error":
                let msg = payload?["message"]?.value as? String ?? payload?["error"]?.value as? String ?? "Unknown error"
                errorBanner = msg.isEmpty ? "Unknown error" : msg
            case "health":
                break // Connection status handled by ConnectionManager state
            case "chat":
                let state = payload?["state"]?.value as? String
                let content = payload?["message"]?.value as? String
                if let content = content, !content.isEmpty {
                    switch state {
                    case "received":
                        messages.append(ChatMessage(id: UUID(), text: content, role: .user))
                    case "sent":
                        messages.append(ChatMessage(id: UUID(), text: content, role: .assistant))
                    case "chunk":
                        if let last = messages.last, last.role == .assistant {
                            messages[messages.count - 1] = ChatMessage(
                                id: last.id, text: last.text + content, role: .assistant
                            )
                        } else {
                            messages.append(ChatMessage(id: UUID(), text: content, role: .assistant))
                        }
                    default:
                        break
                    }
                }
            case "agent.tool":
                let name = payload?["message"]?.value as? String ?? payload?["tool"]?.value as? String ?? payload?["name"]?.value as? String ?? "tool"
                let args = (payload?["arguments"]?.value as? [String: Any])
                    .flatMap { try? JSONSerialization.data(withJSONObject: $0) }
                    .flatMap { String(data: $0, encoding: .utf8) }
                if let ok = payload?["success"]?.value as? Bool, !toolCalls.isEmpty {
                    let idx = toolCalls.count - 1
                    let result = (payload?["detail"]?.value ?? payload?["message"]?.value).map { "\($0)" }
                    toolCalls[idx] = ToolCallInfo(
                        id: toolCalls[idx].id,
                        name: toolCalls[idx].name,
                        arguments: toolCalls[idx].arguments,
                        status: ok ? .completed : .failed,
                        result: result
                    )
                } else {
                    toolCalls.append(ToolCallInfo(
                        id: UUID(),
                        name: name,
                        arguments: args,
                        status: .running
                    ))
                }
            default:
                break
            }
        }
    }
}

struct ChatMessage: Identifiable {
    let id: UUID
    let text: String
    let role: ChatBubble.Role
}

struct ToolCallInfo: Identifiable {
    let id: UUID
    let name: String
    let arguments: String?
    var status: ToolCallCard.Status
    var result: String?
}
