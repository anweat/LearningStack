## SpringMVC 快速查询与总体概览

> 本文作为 **SpringMVC 学习的速查表（cheatsheet）**，方便你在阅读其他文档或写代码时，快速查注解、常用配置、核心组件关系与典型代码模板。

---

### 1. 框架总体构成速览

- **核心角色**：

  - **DispatcherServlet**：前端控制器，请求统一入口。
  - **HandlerMapping**：根据 URL/HTTP 方法 找到对应 Handler（Controller 方法）。
  - **HandlerAdapter**：适配不同类型 Handler，使其可被统一调用。
  - **HandlerMethodArgumentResolver**：方法参数解析（`@RequestParam`、`@PathVariable`、`@RequestBody` 等）。
  - **HandlerMethodReturnValueHandler**：返回值处理（视图名、JSON 等）。
  - **ViewResolver / View**：视图解析与渲染。
  - **HandlerInterceptor**：请求前后拦截（权限、日志等）。

- **典型调用链（REST 场景）**：
  - `客户端 -> DispatcherServlet -> HandlerMapping -> HandlerAdapter -> Controller 方法 -> ReturnValueHandler/HttpMessageConverter -> 响应`

---

### 2. 常用注解速查（Controller & 映射）

- **类级别**：

  - `@Controller`：声明控制器（返回视图为主，配合 `@ResponseBody` 可返回 JSON）。
  - `@RestController`：`@Controller + @ResponseBody` 组合，默认返回 JSON。
  - `@RequestMapping("/api/users")`：定义控制器的基础路径。

- **方法级别映射**：

  - `@GetMapping("/{id}")`：GET `/api/users/{id}`。
  - `@PostMapping`：POST `/api/users`。
  - `@PutMapping("/{id}")`：PUT `/api/users/{id}`。
  - `@DeleteMapping("/{id}")`：DELETE `/api/users/{id}`。
  - `@RequestMapping(value = "/search", method = RequestMethod.GET)`：通用写法。

- **参数绑定**：

  - `@RequestParam("name") String name`：绑定查询参数 `?name=xxx`。
  - `@PathVariable("id") Long id`：绑定路径变量 `/users/{id}`。
  - `@RequestBody CreateUserRequest req`：从请求体 JSON 反序列化。

- **返回值相关**：
  - `@ResponseBody`：将返回值写入 HTTP 响应体（走 HttpMessageConverter）。
  - `ResponseEntity<T>`：携带 HTTP 状态码与 Headers 的响应封装。

---

### 3. 校验与异常处理速查

- **参数校验注解（JSR-303/380）**：
  - `@NotNull`, `@NotBlank`, `@Size`, `@Email`, `@Min`, `@Max` 等。
- **Controller 中启用校验**：
  - `public ApiResponse<UserDto> create(@Valid @RequestBody CreateUserRequest req)`。
- **全局异常处理**：
  - `@ControllerAdvice`：声明全局异常处理类。
  - `@ExceptionHandler(BusinessException.class)`：指定处理某种异常。

```java
@ControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(BusinessException.class)
    @ResponseBody
    public ResponseEntity<ApiResponse<Void>> handleBusiness(BusinessException ex) {
        return new ResponseEntity<>(
                ApiResponse.error(ex.getCode(), ex.getMessage()),
                HttpStatus.BAD_REQUEST
        );
    }
}
```

---

### 4. 配置速查（基础 SpringMVC 项目）

#### 4.1 web.xml 中注册 DispatcherServlet

```xml
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

#### 4.2 spring-mvc.xml 基本配置

```xml
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

    <!-- 视图解析器（如果使用 JSP 等） -->
    <!--
    <bean class="org.springframework.web.servlet.view.InternalResourceViewResolver">
        <property name="prefix" value="/WEB-INF/views/"/>
        <property name="suffix" value=".jsp"/>
    </bean>
    -->
</beans>
```

---

### 5. 拦截器与过滤器速查

- **注册拦截器（XML 方式）**：

```xml
<mvc:interceptors>
    <bean class="com.example.web.interceptor.LogInterceptor"/>
</mvc:interceptors>
```

- **拦截器接口关键方法**：
  - `preHandle()`：Controller 之前。
  - `postHandle()`：Controller 之后、视图渲染前。
  - `afterCompletion()`：视图渲染后（资源清理）。

```java
public class LogInterceptor implements HandlerInterceptor {
    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) {
        // 日志、鉴权等
        return true;
    }
}
```

- **Filter 与 Interceptor 的简单对比**：
  - Filter：Servlet 规范层面，和 Spring 无关，适合做跨项目/跨框架的通用过滤。
  - Interceptor：SpringMVC 层面，可拿到 Handler 信息，适合做接口级别日志、权限控制等。

---

### 6. 统一响应模型与常用模板

- **统一响应类**：

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

- **典型 REST Controller 模板**：

```java
@Controller
@RequestMapping("/api/users")
public class UserController {

    @Autowired
    private UserService userService;

    @GetMapping("/{id}")
    @ResponseBody
    public ApiResponse<UserDto> detail(@PathVariable Long id) {
        return ApiResponse.success(userService.getById(id));
    }
}
```

---

### 7. 学习时如何使用本速查表

- **配合其他文档**：
  - 看到某个概念（如 `HandlerInterceptor`、`@ControllerAdvice`），可以先来本文件找到**最小示例 + 关键配置**。
  - 需要写 CRUD、REST 接口时，直接从“典型模板”中复制一份再按业务改。
- **与未来 Spring Boot 学习的关系**：
  - Boot 会帮你简化很多 XML/Servlet 配置，但底层概念几乎一致：
    - 这些注解、扩展点、拦截器/过滤器用法，在 Boot 中仍然适用，只是换成自动配置或 Java 配置。
