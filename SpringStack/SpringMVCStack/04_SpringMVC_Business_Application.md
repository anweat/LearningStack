## 04：SpringMVC 在具体业务中的应用

> 本篇用“用户管理 REST 模块”为例，演示在基础 SpringMVC 上如何按三层架构组织代码，并为后续拆分为微服务做铺垫。

### 4.1 典型业务场景：用户管理模块示例

- **功能需求**（示例）：
  - 用户注册 / 登录
  - 查询用户列表、查看详情
  - 修改资料、禁用账号
- **设计思路**：
  - 按资源设计 URL：`/api/users`、`/api/users/{id}`。
  - 拆分为：`UserController` + `UserService` + `UserRepository`。

#### 4.1.1 UserController 示例（REST API）

```java
@Controller
@RequestMapping("/api/users")
public class UserController {

    @Autowired
    private UserService userService;

    @GetMapping
    @ResponseBody
    public ApiResponse<List<UserDto>> list() {
        return ApiResponse.success(userService.listUsers());
    }

    @GetMapping("/{id}")
    @ResponseBody
    public ApiResponse<UserDto> detail(@PathVariable Long id) {
        return ApiResponse.success(userService.getById(id));
    }
}
```

### 4.2 三层架构在 SpringMVC 中的落地

- **Controller 层**：
  - 接收 HTTP 请求、解析参数。
  - 调用 Service，处理异常并转换为 HTTP 响应。
- **Service 层**：
  - 承载业务规则，如注册时的唯一性校验、密码加密等。
- **Repository 层**：
  - 仅关心数据持久化细节，如使用 JPA、MyBatis 等。

```java
public interface UserService {
    List<UserDto> listUsers();
    UserDto getById(Long id);
}

@Service
public class UserServiceImpl implements UserService {

    @Autowired
    private UserRepository userRepository;

    @Override
    public List<UserDto> listUsers() {
        List<User> users = userRepository.findAll();
        // 省略实体转 DTO 的细节
        return toDtoList(users);
    }
}
```

### 4.3 RESTful API 设计与状态码使用

- **HTTP 方法语义**：
  - `GET`：查询
  - `POST`：创建
  - `PUT`：整体更新
  - `PATCH`：部分更新
  - `DELETE`：删除
- **返回值设计**：
  - 成功时返回资源本身或操作结果。
  - 失败时返回统一结构的错误响应，包含业务错误码与提示信息。

### 4.4 统一异常处理设计

- 使用 `@ControllerAdvice` + `@ExceptionHandler` 实现：
  - 将系统异常转换为标准错误响应结构。
  - 根据异常类型返回合适的 HTTP 状态码（如 400/404/500 等）。
- 结合日志策略（记录错误栈、请求关键信息）。

```java
@ControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(BusinessException.class)
    @ResponseBody
    public ResponseEntity<ApiResponse<Void>> handleBusiness(BusinessException ex) {
        ApiResponse<Void> body = ApiResponse.error(ex.getCode(), ex.getMessage());
        return new ResponseEntity<>(body, HttpStatus.BAD_REQUEST);
    }

    @ExceptionHandler(Exception.class)
    @ResponseBody
    public ResponseEntity<ApiResponse<Void>> handleOther(Exception ex) {
        ApiResponse<Void> body = ApiResponse.error(500, "Internal Server Error");
        return new ResponseEntity<>(body, HttpStatus.INTERNAL_SERVER_ERROR);
    }
}
```

### 4.5 业务中的常见横切关注点

- **认证与授权**：
  - 基于拦截器或 Spring Security 进行登录态校验与权限校验。
- **审计与操作日志**：
  - 记录用户行为、敏感操作、接口调用耗时等。
- **防重复提交 / 幂等性设计**：
  - 结合 Token、业务幂等键（如订单号）等机制。

### 4.6 面向业务的包结构与模块划分

- **按“业务域”组织包**（推荐）：
  - 如：`user`, `order`, `product` 等，每个包内部自包含 controller/service/repository/model。
- **好处**：
  - 模块边界清晰，便于拆分与重构。
  - 新人易于理解与定位代码。

### 4.7 从 Demo 到“可维护系统”

- 评价一个 SpringMVC 项目是否设计良好，不仅看功能是否能跑：
  - **依赖方向是否清晰**（Controller -> Service -> Repository）。
  - **业务规则是否集中在 Service，而非四处散落**。
  - **异常与日志是否统一处理**，方便排查问题。
