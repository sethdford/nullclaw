// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "SeaClawiOS",
    platforms: [.iOS(.v17), .macOS(.v14)],
    products: [
        .library(name: "SeaClawiOS", targets: ["SeaClawiOS"]),
    ],
    dependencies: [
        .package(path: "../shared/SeaClawKit"),
    ],
    targets: [
        .target(
            name: "SeaClawiOS",
            dependencies: [
                .product(name: "SeaClawClient", package: "SeaClawKit"),
                .product(name: "SeaClawChatUI", package: "SeaClawKit"),
                .product(name: "SeaClawProtocol", package: "SeaClawKit"),
            ],
            path: "Sources/SeaClawiOS"
        ),
    ]
)
