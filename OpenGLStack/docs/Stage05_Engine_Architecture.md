# 阶段5: 渲染引擎架构

## 🎯 学习目标

- 设计可扩展的渲染引擎架构
- 实现资源管理系统
- 构建场景图和ECS架构
- 实现渲染队列和批处理
- 掌握多线程渲染技术
- 设计材质系统和渲染管线

## 📋 框架性问题指引

### 核心问题1: 什么是好的引擎架构？
**问题分解:**
- 如何分离关注点（渲染、逻辑、资源）？
- 如何设计可扩展的系统？
- 如何平衡性能和可维护性？

**设计原则:**
```
- 单一职责原则（SRP）
- 开闭原则（OCP）
- 依赖倒置原则（DIP）
- 数据驱动设计
```

### 核心问题2: 场景图vs ECS哪个更好？
**问题分解:**
- 场景图的优缺点？
- ECS的核心思想？
- 如何选择合适的架构？

**对比:**
```
场景图: 树形层级，适合简单场景，易于理解
ECS: 组件化设计，缓存友好，适合大规模实体
```

### 核心问题3: 如何优化渲染性能？
**问题分解:**
- 批处理的原理？
- 如何减少状态切换？
- 多线程渲染的挑战？

## 💻 核心概念详解

### 1. 引擎整体架构

#### 模块划分

```cpp
// 引擎核心架构
namespace Engine {

// 核心层
class Core {
public:
    void Initialize();
    void Run();
    void Shutdown();
    
private:
    Window* m_Window;
    Renderer* m_Renderer;
    ResourceManager* m_ResourceManager;
    SceneManager* m_SceneManager;
    InputManager* m_InputManager;
    TimeManager* m_TimeManager;
};

// 渲染层
class Renderer {
public:
    void BeginFrame();
    void EndFrame();
    void Submit(RenderCommand* command);
    void Flush();
    
private:
    RenderQueue m_RenderQueue;
    RenderContext* m_Context;
};

// 资源层
class ResourceManager {
public:
    template<typename T>
    std::shared_ptr<T> Load(const std::string& path);
    
    template<typename T>
    void Unload(const std::string& path);
    
    void UnloadUnused();
    
private:
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_Resources;
    std::mutex m_Mutex;
};

// 场景层
class SceneManager {
public:
    void LoadScene(const std::string& name);
    void UnloadScene();
    Scene* GetActiveScene();
    
private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_Scenes;
    Scene* m_ActiveScene;
};

} // namespace Engine
```

### 2. 资源管理系统

#### 资源基类

```cpp
class Resource {
public:
    enum class Type {
        Texture, Mesh, Shader, Material, Audio
    };
    
    virtual ~Resource() = default;
    virtual Type GetType() const = 0;
    virtual void Load(const std::string& path) = 0;
    virtual void Unload() = 0;
    
    const std::string& GetPath() const { return m_Path; }
    bool IsLoaded() const { return m_Loaded; }
    
    void AddReference() { ++m_RefCount; }
    void RemoveReference() { --m_RefCount; }
    int GetRefCount() const { return m_RefCount; }
    
protected:
    std::string m_Path;
    bool m_Loaded = false;
    std::atomic<int> m_RefCount{0};
};

// 纹理资源
class Texture : public Resource {
public:
    struct Parameters {
        GLenum wrapS = GL_REPEAT;
        GLenum wrapT = GL_REPEAT;
        GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
        GLenum magFilter = GL_LINEAR;
        bool generateMipmaps = true;
    };
    
    Type GetType() const override { return Type::Texture; }
    
    void Load(const std::string& path) override {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        
        if (data) {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
            
            glGenTextures(1, &m_ID);
            glBindTexture(GL_TEXTURE_2D, m_ID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            
            if (m_Params.generateMipmaps)
                glGenerateMipmap(GL_TEXTURE_2D);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_Params.wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_Params.wrapT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Params.minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Params.magFilter);
            
            m_Width = width;
            m_Height = height;
            m_Channels = channels;
            m_Loaded = true;
            
            stbi_image_free(data);
        }
        
        m_Path = path;
    }
    
    void Unload() override {
        if (m_Loaded) {
            glDeleteTextures(1, &m_ID);
            m_Loaded = false;
        }
    }
    
    void Bind(unsigned int slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_ID);
    }
    
private:
    unsigned int m_ID = 0;
    int m_Width = 0, m_Height = 0, m_Channels = 0;
    Parameters m_Params;
};

// 网格资源
class Mesh : public Resource {
public:
    Type GetType() const override { return Type::Mesh; }
    
    void Load(const std::string& path) override {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        
        // 处理网格...
        m_Loaded = true;
        m_Path = path;
    }
    
    void Unload() override {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        m_Loaded = false;
    }
    
    void Draw() const {
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
private:
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_IndexCount = 0;
};
```

#### 资源管理器实现

```cpp
class ResourceManager {
public:
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }
    
    template<typename T>
    std::shared_ptr<T> Load(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        // 检查是否已加载
        auto it = m_Resources.find(path);
        if (it != m_Resources.end()) {
            if (auto resource = std::dynamic_pointer_cast<T>(it->second)) {
                resource->AddReference();
                return resource;
            }
        }
        
        // 加载新资源
        auto resource = std::make_shared<T>();
        resource->Load(path);
        resource->AddReference();
        m_Resources[path] = resource;
        
        return resource;
    }
    
    template<typename T>
    void Unload(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        auto it = m_Resources.find(path);
        if (it != m_Resources.end()) {
            it->second->RemoveReference();
            if (it->second->GetRefCount() <= 0) {
                it->second->Unload();
                m_Resources.erase(it);
            }
        }
    }
    
    void UnloadUnused() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        for (auto it = m_Resources.begin(); it != m_Resources.end();) {
            if (it->second->GetRefCount() <= 0) {
                it->second->Unload();
                it = m_Resources.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void UnloadAll() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        for (auto& pair : m_Resources) {
            pair.second->Unload();
        }
        m_Resources.clear();
    }
    
private:
    ResourceManager() = default;
    ~ResourceManager() { UnloadAll(); }
    
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_Resources;
    std::mutex m_Mutex;
};

// 使用示例
auto texture = ResourceManager::Get().Load<Texture>("textures/wall.jpg");
auto mesh = ResourceManager::Get().Load<Mesh>("models/cube.obj");
```

### 3. ECS架构实现

#### 组件系统

```cpp
// 组件基类
struct Component {
    virtual ~Component() = default;
};

// 具体组件
struct TransformComponent : Component {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    
    glm::mat4 GetModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, scale);
        return model;
    }
};

struct MeshRendererComponent : Component {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
};

struct CameraComponent : Component {
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    bool isPrimary = false;
    
    glm::mat4 GetProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
};

struct LightComponent : Component {
    enum class Type { Directional, Point, Spot };
    
    Type type = Type::Point;
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    
    // Point/Spot light
    float range = 10.0f;
    
    // Spot light
    float innerConeAngle = 30.0f;
    float outerConeAngle = 45.0f;
};
```

#### 实体管理

```cpp
using EntityID = uint32_t;

class Entity {
public:
    Entity(EntityID id) : m_ID(id) {}
    
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        m_Components[typeid(T).hash_code()] = component;
        return *component;
    }
    
    template<typename T>
    T* GetComponent() {
        auto it = m_Components.find(typeid(T).hash_code());
        if (it != m_Components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    bool HasComponent() const {
        return m_Components.find(typeid(T).hash_code()) != m_Components.end();
    }
    
    template<typename T>
    void RemoveComponent() {
        m_Components.erase(typeid(T).hash_code());
    }
    
    EntityID GetID() const { return m_ID; }
    
private:
    EntityID m_ID;
    std::unordered_map<size_t, std::shared_ptr<Component>> m_Components;
};

class EntityManager {
public:
    Entity* CreateEntity() {
        EntityID id = m_NextID++;
        auto entity = std::make_unique<Entity>(id);
        Entity* ptr = entity.get();
        m_Entities[id] = std::move(entity);
        return ptr;
    }
    
    void DestroyEntity(EntityID id) {
        m_Entities.erase(id);
    }
    
    Entity* GetEntity(EntityID id) {
        auto it = m_Entities.find(id);
        return (it != m_Entities.end()) ? it->second.get() : nullptr;
    }
    
    template<typename T>
    std::vector<Entity*> GetEntitiesWithComponent() {
        std::vector<Entity*> result;
        for (auto& pair : m_Entities) {
            if (pair.second->HasComponent<T>()) {
                result.push_back(pair.second.get());
            }
        }
        return result;
    }
    
private:
    std::unordered_map<EntityID, std::unique_ptr<Entity>> m_Entities;
    EntityID m_NextID = 0;
};
```

#### 系统基类

```cpp
class System {
public:
    virtual ~System() = default;
    virtual void Update(float deltaTime, EntityManager& entityManager) = 0;
};

// 渲染系统
class RenderSystem : public System {
public:
    void Update(float deltaTime, EntityManager& entityManager) override {
        // 获取相机
        CameraComponent* camera = nullptr;
        Entity* cameraEntity = nullptr;
        for (auto* entity : entityManager.GetEntitiesWithComponent<CameraComponent>()) {
            auto* cam = entity->GetComponent<CameraComponent>();
            if (cam->isPrimary) {
                camera = cam;
                cameraEntity = entity;
                break;
            }
        }
        
        if (!camera || !cameraEntity) return;
        
        // 设置视图和投影矩阵
        auto* camTransform = cameraEntity->GetComponent<TransformComponent>();
        glm::mat4 view = glm::lookAt(
            camTransform->position,
            camTransform->position + glm::vec3(0, 0, -1),  // 简化版
            glm::vec3(0, 1, 0)
        );
        glm::mat4 projection = camera->GetProjectionMatrix(16.0f / 9.0f);
        
        // 渲染所有mesh
        for (auto* entity : entityManager.GetEntitiesWithComponent<MeshRendererComponent>()) {
            auto* transform = entity->GetComponent<TransformComponent>();
            auto* renderer = entity->GetComponent<MeshRendererComponent>();
            
            if (!transform || !renderer || !renderer->mesh || !renderer->material) continue;
            
            // 绑定材质
            renderer->material->Bind();
            renderer->material->SetMat4("model", transform->GetModelMatrix());
            renderer->material->SetMat4("view", view);
            renderer->material->SetMat4("projection", projection);
            
            // 绘制
            renderer->mesh->Draw();
        }
    }
};

// 使用示例
EntityManager entityManager;
RenderSystem renderSystem;

// 创建实体
Entity* cube = entityManager.CreateEntity();
auto& transform = cube->AddComponent<TransformComponent>();
transform.position = glm::vec3(0.0f, 0.0f, -5.0f);

auto& renderer = cube->AddComponent<MeshRendererComponent>();
renderer.mesh = ResourceManager::Get().Load<Mesh>("models/cube.obj");
renderer.material = std::make_shared<Material>();

// 更新循环
while (running) {
    renderSystem.Update(deltaTime, entityManager);
}
```

### 4. 渲染队列与批处理

#### 渲染命令

```cpp
struct RenderCommand {
    glm::mat4 transform;
    Mesh* mesh;
    Material* material;
    float distanceToCamera;  // 用于排序
    
    // 排序键（减少状态切换）
    uint64_t GetSortKey() const {
        uint64_t key = 0;
        key |= (uint64_t)(material->GetShaderID()) << 32;
        key |= (uint64_t)(material->GetTextureID()) << 16;
        key |= (uint64_t)(mesh->GetVAO());
        return key;
    }
};

class RenderQueue {
public:
    void Submit(const RenderCommand& command) {
        m_Commands.push_back(command);
    }
    
    void Sort() {
        // 不透明物体：前到后（早期深度测试）
        std::sort(m_OpaqueCommands.begin(), m_OpaqueCommands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                return a.GetSortKey() < b.GetSortKey();
            });
        
        // 透明物体：后到前（混合正确）
        std::sort(m_TransparentCommands.begin(), m_TransparentCommands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                return a.distanceToCamera > b.distanceToCamera;
            });
    }
    
    void Execute() {
        for (auto& cmd : m_OpaqueCommands) {
            cmd.material->Bind();
            cmd.material->SetMat4("model", cmd.transform);
            cmd.mesh->Draw();
        }
        
        for (auto& cmd : m_TransparentCommands) {
            cmd.material->Bind();
            cmd.material->SetMat4("model", cmd.transform);
            cmd.mesh->Draw();
        }
    }
    
    void Clear() {
        m_OpaqueCommands.clear();
        m_TransparentCommands.clear();
    }
    
private:
    std::vector<RenderCommand> m_OpaqueCommands;
    std::vector<RenderCommand> m_TransparentCommands;
};
```

#### 实例化渲染

```cpp
class InstancedRenderer {
public:
    void AddInstance(const glm::mat4& transform) {
        m_Transforms.push_back(transform);
    }
    
    void Render(Mesh* mesh, Material* material) {
        if (m_Transforms.empty()) return;
        
        // 更新实例缓冲
        if (m_InstanceVBO == 0) {
            glGenBuffers(1, &m_InstanceVBO);
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, m_InstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, m_Transforms.size() * sizeof(glm::mat4),
                     m_Transforms.data(), GL_DYNAMIC_DRAW);
        
        // 设置实例属性
        glBindVertexArray(mesh->GetVAO());
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                                  (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(3 + i, 1);
        }
        
        // 绘制
        material->Bind();
        glDrawElementsInstanced(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0,
                                m_Transforms.size());
        
        m_Transforms.clear();
    }
    
private:
    std::vector<glm::mat4> m_Transforms;
    unsigned int m_InstanceVBO = 0;
};
```

### 5. 材质系统

```cpp
class Material {
public:
    Material(std::shared_ptr<Shader> shader) : m_Shader(shader) {}
    
    void SetTexture(const std::string& name, std::shared_ptr<Texture> texture) {
        m_Textures[name] = texture;
    }
    
    void SetFloat(const std::string& name, float value) {
        m_Floats[name] = value;
    }
    
    void SetVec3(const std::string& name, const glm::vec3& value) {
        m_Vec3s[name] = value;
    }
    
    void SetMat4(const std::string& name, const glm::mat4& value) {
        m_Mat4s[name] = value;
    }
    
    void Bind() {
        m_Shader->use();
        
        int textureSlot = 0;
        for (auto& pair : m_Textures) {
            pair.second->Bind(textureSlot);
            m_Shader->setInt(pair.first, textureSlot);
            ++textureSlot;
        }
        
        for (auto& pair : m_Floats)
            m_Shader->setFloat(pair.first, pair.second);
        
        for (auto& pair : m_Vec3s)
            m_Shader->setVec3(pair.first, pair.second);
        
        for (auto& pair : m_Mat4s)
            m_Shader->setMat4(pair.first, pair.second);
    }
    
    unsigned int GetShaderID() const { return m_Shader->ID; }
    unsigned int GetTextureID() const {
        return m_Textures.empty() ? 0 : m_Textures.begin()->second->GetID();
    }
    
private:
    std::shared_ptr<Shader> m_Shader;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
    std::unordered_map<std::string, float> m_Floats;
    std::unordered_map<std::string, glm::vec3> m_Vec3s;
    std::unordered_map<std::string, glm::mat4> m_Mat4s;
};
```

## 💡 常见问题解答

### Q1: 何时使用场景图，何时使用ECS？
- **场景图**: 小型项目、简单层级关系、快速原型
- **ECS**: 大型游戏、大量实体、需要高性能

### Q2: 如何避免过度设计？
- 从简单开始，逐步重构
- 根据实际需求添加功能
- 保持代码可读性

### Q3: 多线程渲染的挑战？
- OpenGL上下文绑定到单线程
- 使用命令队列分离逻辑和渲染
- Vulkan/DX12天然支持多线程

## 📝 学习检查清单

- [ ] 设计模块化引擎架构
- [ ] 实现资源管理系统
- [ ] 构建ECS或场景图
- [ ] 实现渲染队列
- [ ] 掌握批处理和实例化
- [ ] 设计材质系统

## 🚀 下一步

进入 [阶段6: 引擎优化与高级特性](./Stage06_Optimization_Advanced.md)
