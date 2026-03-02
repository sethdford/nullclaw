import SwiftUI

/// Card displaying tool call status (running, completed, failed).
public struct ToolCallCard: View {
    public enum Status {
        case running
        case completed
        case failed
    }

    public let name: String
    public let arguments: String?
    public let status: Status
    public let result: String?

    public init(name: String, arguments: String? = nil, status: Status, result: String? = nil) {
        self.name = name
        self.arguments = arguments
        self.status = status
        self.result = result
    }

    public var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(spacing: 8) {
                Image(systemName: statusIcon)
                    .foregroundStyle(statusColor)
                Text(name)
                    .font(.subheadline.weight(.semibold))
                Spacer()
                if status == .running {
                    ProgressView()
                        .scaleEffect(0.8)
                }
            }

            if let args = arguments, !args.isEmpty {
                Text(args)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
            }

            if let res = result, !res.isEmpty {
                Text(res)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(3)
            }
        }
        .padding(12)
        .background(Color(white: 0.95))
        .clipShape(RoundedRectangle(cornerRadius: 12, style: .continuous))
    }

    private var statusIcon: String {
        switch status {
        case .running: return "gearshape.2"
        case .completed: return "checkmark.circle.fill"
        case .failed: return "exclamationmark.triangle.fill"
        }
    }

    private var statusColor: Color {
        switch status {
        case .running: return .orange
        case .completed: return .green
        case .failed: return .red
        }
    }
}

#Preview {
    VStack(spacing: 12) {
        ToolCallCard(name: "shell", arguments: "{\"cmd\":\"ls\"}", status: .running)
        ToolCallCard(name: "browser", status: .completed, result: "Opened page")
    }
    .padding()
}
