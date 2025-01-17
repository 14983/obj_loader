# obj\_loader

---

## 依赖

+ `libglm-dev`
+ `libglfw3-dev`
+ `libimgui-dev`
+ `libglew-dev`

## 文件说明

+ `res/model` 下的 `.obj` 文件并非本人编写
+ `imgui_util` 用于封装 ImGui 相关的功能
+ `mtllib` 用于解析 `.mtl` 文件，辅助 `obj_loader` 渲染
+ `obj_loader` 用于加载 `.obj` 文件并渲染
+ `main.cpp` 主函数，用于测试 `obj_loader`

## 目前实现的功能

+ 支持 `.obj` 文件的加载、渲染、更改材质、变换、保存，**仅支持以 group 分隔，一个 group 只能绑定一个材质**
+ 支持 `.mtl` 文件的解析，但**不支持纹理贴图**
+ 支持光源属性的设置，包括颜色、位置

## 编译运行

```bash
make
./main
```
