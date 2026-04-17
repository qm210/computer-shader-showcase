import AppKit
import Metal
import MetalKit
import simd

let WIDTH = 400
let HEIGHT = 300
let PIXEL_SCALE: CGFloat = 3.0
let FPS: Double = 15.0
let INIT_ALIVE_CHANCE: Double = 0.2
let INIT_RANDOM_SEED: UInt64 = 0

struct GridSize {
    var width: UInt32
    var height: UInt32
}

final class Renderer: NSObject, MTKViewDelegate {
    let device: MTLDevice
    let queue: MTLCommandQueue
    let computePipeline: MTLComputePipelineState
    let renderPipeline: MTLRenderPipelineState
    var stateTextures: [MTLTexture] = []
    var frameIndex: Int = 0
    var paused = false
    var spaceAlreadyPressed = false
    var lastStepTime = CACurrentMediaTime()

    init?(mtkView: MTKView) {
        guard let device = MTLCreateSystemDefaultDevice(),
              let queue = device.makeCommandQueue() else {
            fputs("No Metal device available\n", stderr)
            return nil
        }
        self.device = device
        self.queue = queue
        mtkView.device = device
        mtkView.colorPixelFormat = .bgra8Unorm
        mtkView.framebufferOnly = false
        mtkView.clearColor = MTLClearColorMake(0.08, 0.08, 0.08, 1.0)
        mtkView.preferredFramesPerSecond = Int(FPS)

        let resourceURL = Bundle.module.url(forResource: "Shaders", withExtension: "metallib")!
        guard let library = try? device.makeLibrary(URL: resourceURL) else {
            fputs("Failed to load Shaders.metallib\n", stderr)
            return nil
        }
        guard let computeFn = library.makeFunction(name: "stepLife"),
              let vertexFn = library.makeFunction(name: "fullscreenVertex"),
              let fragmentFn = library.makeFunction(name: "lifeFragment") else {
            fputs("Failed to load shader functions\n", stderr)
            return nil
        }
        do {
            computePipeline = try device.makeComputePipelineState(function: computeFn)
        } catch {
            fputs("Failed to create compute pipeline: \(error)\n", stderr)
            return nil
        }

        let rpd = MTLRenderPipelineDescriptor()
        rpd.vertexFunction = vertexFn
        rpd.fragmentFunction = fragmentFn
        rpd.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        do {
            renderPipeline = try device.makeRenderPipelineState(descriptor: rpd)
        } catch {
            fputs("Failed to create render pipeline: \(error)\n", stderr)
            return nil
        }

        super.init()
        stateTextures = [makeStateTexture(initial: true), makeStateTexture(initial: false)]
    }

    func makeStateTexture(initial: Bool) -> MTLTexture {
        let desc = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: .r8Uint, width: WIDTH, height: HEIGHT, mipmapped: false)
        desc.usage = [.shaderRead, .shaderWrite]
        desc.storageMode = .managed
        let tex = device.makeTexture(descriptor: desc)!

        var data = [UInt8](repeating: 0, count: WIDTH * HEIGHT)
        if initial {
            var rng = SeededGenerator(seed: INIT_RANDOM_SEED)
            for i in data.indices {
                data[i] = Double.random(in: 0..<1, using: &rng) < INIT_ALIVE_CHANCE ? 1 : 0
            }
        }
        tex.replace(region: MTLRegionMake2D(0, 0, WIDTH, HEIGHT), mipmapLevel: 0, withBytes: data, bytesPerRow: WIDTH)
        tex.didModifyRange(0..<tex.allocatedSize)
        return tex
    }

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {}

    func draw(in view: MTKView) {
        guard let drawable = view.currentDrawable,
              let rpd = view.currentRenderPassDescriptor else { return }

        let spacePressed = NSEvent.modifierFlags.contains([]) && (NSApp.currentEvent?.type == .keyDown && NSApp.currentEvent?.keyCode == 49)
        if spacePressed && !spaceAlreadyPressed { paused.toggle() }
        spaceAlreadyPressed = spacePressed

        let now = CACurrentMediaTime()
        let shouldStep = !paused && (now - lastStepTime >= 1.0 / FPS)
        if shouldStep { lastStepTime = now }

        guard let commandBuffer = queue.makeCommandBuffer() else { return }

        var displayIndex = frameIndex % 2
        if shouldStep {
            let ping = frameIndex % 2
            let pong = 1 - ping
            var grid = GridSize(width: UInt32(WIDTH), height: UInt32(HEIGHT))

            if let encoder = commandBuffer.makeComputeCommandEncoder() {
                encoder.setComputePipelineState(computePipeline)
                encoder.setTexture(stateTextures[ping], index: 0)
                encoder.setTexture(stateTextures[pong], index: 1)
                encoder.setBytes(&grid, length: MemoryLayout<GridSize>.stride, index: 0)

                let tg = MTLSize(width: 16, height: 16, depth: 1)
                let ng = MTLSize(width: (WIDTH + 15) / 16, height: (HEIGHT + 15) / 16, depth: 1)
                encoder.dispatchThreadgroups(ng, threadsPerThreadgroup: tg)
                encoder.endEncoding()
            }
            displayIndex = pong
            frameIndex += 1
        }

        if let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: rpd) {
            encoder.setRenderPipelineState(renderPipeline)
            encoder.setFragmentTexture(stateTextures[displayIndex], index: 0)
            encoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
            encoder.endEncoding()
        }

        commandBuffer.present(drawable)
        commandBuffer.commit()
    }
}

struct SeededGenerator: RandomNumberGenerator {
    private var state: UInt64
    init(seed: UInt64) { self.state = seed == 0 ? 0x123456789abcdef : seed }
    mutating func next() -> UInt64 {
        state ^= state >> 12
        state ^= state << 25
        state ^= state >> 27
        return state &* 2685821657736338717
    }
}

final class AppDelegate: NSObject, NSApplicationDelegate {
    var window: NSWindow!
    var metalView: MTKView!
    var renderer: Renderer!

    func applicationDidFinishLaunching(_ notification: Notification) {
        let size = NSSize(width: CGFloat(WIDTH) * PIXEL_SCALE, height: CGFloat(HEIGHT) * PIXEL_SCALE)
        window = NSWindow(
            contentRect: NSRect(origin: .zero, size: size),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false)
        window.title = "Space to pause"
        window.center()

        metalView = MTKView(frame: NSRect(origin: .zero, size: size))
        guard let renderer = Renderer(mtkView: metalView) else {
            NSApp.terminate(nil)
            return
        }
        self.renderer = renderer
        metalView.delegate = renderer
        metalView.enableSetNeedsDisplay = false
        metalView.isPaused = false
        window.contentView = metalView
        window.makeKeyAndOrderFront(nil)
        window.makeFirstResponder(window)
    }
}

let app = NSApplication.shared
let delegate = AppDelegate()
app.setActivationPolicy(.regular)
app.delegate = delegate
app.activate(ignoringOtherApps: true)
app.run()
