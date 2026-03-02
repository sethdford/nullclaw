import SwiftUI

struct SettingsView: View {
    @EnvironmentObject var connectionManager: ConnectionManager

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("Gateway URL", text: $connectionManager.gatewayURL)
                        .autocorrectionDisabled()
#if os(iOS)
                        .textInputAutocapitalization(.never)
                        .keyboardType(.URL)
#endif
                } header: {
                    Text("Connection")
                } footer: {
                    Text("WebSocket URL, e.g. wss://localhost:8080/ws")
                }

                Section {
                    HStack {
                        Text("Status")
                        Spacer()
                        HStack(spacing: 6) {
                            Circle()
                                .fill(connectionManager.isConnected ? Color.green : Color.red)
                                .frame(width: 8, height: 8)
                            Text(connectionManager.isConnected ? "Connected" : "Disconnected")
                                .foregroundStyle(.secondary)
                        }
                    }

                    Button(connectionManager.isConnected ? "Disconnect" : "Connect") {
                        if connectionManager.isConnected {
                            connectionManager.disconnect()
                        } else {
                            connectionManager.connect()
                        }
                    }

                    if connectionManager.isConnected {
                        Button("Reconnect") {
                            connectionManager.reconnect()
                        }
                    }
                } header: {
                    Text("Connection Status")
                }
            }
            .navigationTitle("Settings")
        }
    }
}
