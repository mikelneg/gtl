# gtl
## graphics tools library

[![gtl-demo-app video](https://img.youtube.com/vi/XNLS5wE2rkA/0.jpg)](https://youtu.be/XNLS5wE2rkA)

Early phase demo; major interfaces are still WIP.

Currently includes:
* Multi-threaded engine
* Direct3D12 graphics pipeline with RAII wrapper types (central WIP)
* Vertex skinning with Box2D objects as control points
* Object selection/interaction handled via an "id layer" in the graphics pipeline
* Dear imgui overlay (early)
* Audio effects and controller/input support via DirectXTK (early)
* Event handling with boost::coroutines
* Mesh loading with FBXSDK
* Qt adapter (works with Direct3D12, but early and not part of the working demo)
