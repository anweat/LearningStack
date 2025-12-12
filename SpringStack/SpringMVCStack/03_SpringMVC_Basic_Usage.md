## 03：SpringMVC 基本使用与项目骨架

> 本篇围绕“一个最小可用的 SpringMVC REST 项目长什么样”来展开，所有示例都基于**非 Boot 的基础 SpringMVC**，方便后续对比迁移到 Spring Boot。

### 3.1 基础环境与项目结构

- **典型分层**：
  - **web 层（controller）**：只做请求处理与结果组装。
  - **service 层**：承载业务逻辑。
  - **repository/dao 层**：数据访问。
- **推荐包结构示意**：
  - `config/`：Web 配置、拦截器注册等。
  - `controller/`：API 入口。
  - `service/impl/`：业务逻辑实现。
  - `repository/`：JPA/MyBatis 等持久化接口。
  - `model/dto/vo/`：请求与响应数据结构。

#### 3.1.1 最小 SpringMVC XML 配置示例（基于 Servlet 3.x/老项目）

```xml
<!-- web.xml 中注册 DispatcherServlet -->
<servlet>
    <servlet-name>springmvc</servlet-name>
    <servlet-class>org.springframework.web.servlet.DispatcherServlet</servlet-class>
    <init-param>
        <param-name>contextConfigLocation</param-name>
        <param-value>classpath:spring-mvc.xml</param-value>
    </init-param>
    <load-on-startup>1</load-on-startup>
</servlet>

<servlet-mapping>
    <servlet-name>springmvc</servlet-name>
    <url-pattern>/</url-pattern>
</servlet-mapping>
```

```xml
<!-- spring-mvc.xml：开启注解驱动，配置组件扫描 -->
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xmlns:mvc="http://www.springframework.org/schema/mvc"
       xmlns:context="http://www.springframework.org/schema/context"
       xsi:schemaLocation="
        http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd
        http://www.springframework.org/schema/mvc http://www.springframework.org/schema/mvc/spring-mvc.xsd
        http://www.springframework.org/schema/context http://www.springframework.org/schema/context/spring-context.xsd">

    <!-- 组件扫描：Controller、Service 等 -->
    <context:component-scan base-package="com.example"/>

    <!-- 启用注解驱动：RequestMapping、@ResponseBody 等生效 -->
    <mvc:annotation-driven/>
</beans>
```

### 3.2 Controller 设计：从 Servlet 到注解风格

- **核心注解**：
  - `@Controller` / `@ResponseBody`（或 `@RestController` 在新版本中）。
  - `@RequestMapping` / `@GetMapping` / `@PostMapping` 等
  - `@RequestParam` / `@PathVariable` / `@RequestBody`
- **设计原则**：
  - Controller 方法只做**输入校验 + 调用服务 + 输出转换**。
  - 不要在 Controller 中写复杂业务逻辑，更不要直接访问数据库。

#### 3.2.1 一个简单的 REST Controller 示例

```java
@Controller
@RequestMapping("/api/users")
public class UserController {

    @Autowired
    private UserService userService;

    @GetMapping("/{id}")
    @ResponseBody
    public UserDto getUser(@PathVariable("id") Long id) {
        return userService.getById(id);
    }
}
```

### 3.3 请求参数绑定示例

- **基础类型与查询参数**：
  - 自动绑定到方法参数，如 `String`, `int`, `Long`。
- **路径变量**：
  - 使用 `@PathVariable` 绑定 REST 风格 URL 中的变量。
- **JSON 请求体**：
  - 使用 `@RequestBody` 将 JSON 反序列化为 Java 对象。

#### 3.3.1 多种参数绑定示例

```java
@PostMapping
@ResponseBody
public ApiResponse<UserDto> createUser(@RequestParam("name") String name,
                                       @RequestParam(value = "age", required = false) Integer age,
                                       @RequestBody CreateUserRequest request) {
    UserDto user = userService.createUser(request);
    return ApiResponse.success(user);
}
```

### 3.4 表单与基础校验

- **JSR-303/JSR-380 校验注解**：如 `@NotNull`, `@Size`, `@Email`。
- 配合 `@Valid` 或 `@Validated` 使用，实现参数自动校验。
- 可以配合全局异常处理将校验失败统一转为友好的错误响应。

```java
public class CreateUserRequest {

    @NotBlank
    private String name;

    @Email
    private String email;

    // getter/setter
}

@PostMapping
@ResponseBody
public ApiResponse<UserDto> createUser(@Valid @RequestBody CreateUserRequest request) {
    UserDto user = userService.createUser(request);
    return ApiResponse.success(user);
}
```

### 3.5 统一响应模型（可选但推荐）

- 设计一个通用响应包装类，例如：`ApiResponse<T>`，包含：
  - `code`（业务码）
  - `message`
  - `data`
- 好处：
  - 前后端接口风格统一。
  - 便于集中处理错误与日志记录。

```java
public class ApiResponse<T> {
    private int code;
    private String message;
    private T data;

    public static <T> ApiResponse<T> success(T data) {
        ApiResponse<T> resp = new ApiResponse<>();
        resp.code = 0;
        resp.message = "OK";
        resp.data = data;
        return resp;
    }

    public static <T> ApiResponse<T> error(int code, String message) {
        ApiResponse<T> resp = new ApiResponse<>();
        resp.code = code;
        resp.message = message;
        return resp;
    }
}
```

### 3.6 小结：从“写一个 Controller”到“设计一套 Web 层结构”

- 不仅要会写 `@Controller` + `@RequestMapping`，更要：
  - **思考 URL 命名是否清晰、资源化**。
  - **划分好 Controller 职责，避免上帝 Controller**。
  - **保持入参与出参模型简洁、面向使用场景**。

### 3.1 基础环境与项目结构

- **典型分层**：
  - **web 层（controller）**：只做请求处理与结果组装。
  - **service 层**：承载业务逻辑。
  - **repository/dao 层**：数据访问。
- **Spring Boot + SpringMVC 推荐结构示意**：
  - `config/`：Web 配置、拦截器注册等。
  - `controller/`：API 入口。
  - `service/impl/`：业务逻辑实现。
  - `repository/`：JPA/MyBatis 等持久化接口。
  - `model/dto/vo/`：请求与响应数据结构。

### 3.2 Controller 设计：从 Servlet 到注解风格

- **核心注解**：
  - `@Controller` / `@RestController`
  - `@RequestMapping` / `@GetMapping` / `@PostMapping` 等
  - `@RequestParam` / `@PathVariable` / `@RequestBody`
- **设计原则**：
  - Controller 方法只做**输入校验 + 调用服务 + 输出转换**。
  - 不要在 Controller 中写复杂业务逻辑，更不要直接访问数据库。

### 3.3 请求参数绑定示例

- **基础类型与查询参数**：
  - 自动绑定到方法参数，如 `String`, `int`, `Long`。
- **路径变量**：
  - 使用 `@PathVariable` 绑定 REST 风格 URL 中的变量。
- **JSON 请求体**：
  - 使用 `@RequestBody` 将 JSON 反序列化为 Java 对象。

### 3.4 表单与基础校验

- **JSR-303/JSR-380 校验注解**：如 `@NotNull`, `@Size`, `@Email`。
- 配合 `@Valid` 或 `@Validated` 使用，实现参数自动校验。
- 可以配合全局异常处理将校验失败统一转为友好的错误响应。

### 3.5 统一响应模型（可选但推荐）

- 设计一个通用响应包装类，例如：`ApiResponse<T>`，包含：
  - `code`（业务码）
  - `message`
  - `data`
- 好处：
  - 前后端接口风格统一。
  - 便于集中处理错误与日志记录。

### 3.6 小结：从“写一个 Controller”到“设计一套 Web 层结构”

- 不仅要会写 `@RestController` + `@GetMapping`，更要：
  - **思考 URL 命名是否清晰、资源化**。
  - **划分好 Controller 职责，避免上帝 Controller**。
  - **保持入参与出参模型简洁、面向使用场景**。
