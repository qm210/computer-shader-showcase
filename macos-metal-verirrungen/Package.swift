// swift-tools-version: 5.7
import PackageDescription

let package = Package(
    name: "gol-metal-macos",
    platforms: [
        .macOS(.v12)
    ],
    products: [
        .executable(name: "gol-showcase", targets: ["gol-showcase"])
    ],
    targets: [
        .executableTarget(
            name: "gol-showcase",
            path: "Sources/gol-showcase",
            resources: [.copy("Shaders.metallib")],
            linkerSettings: [
                .linkedFramework("AppKit"),
                .linkedFramework("Metal"),
                .linkedFramework("MetalKit")
            ]
        )
    ]
)
