## 02：SpringMVC 核心机制与运行流程

> 本篇从“请求一次 REST API 发生了什么？”这个问题出发，既画出流程，也给出关键代码/伪代码，帮助你把 SpringMVC 的运行过程和真实代码对应起来。

### 2.1 整体请求处理流程（高层视角）

1. 客户端发起 HTTP 请求。
2. 请求到达 Servlet 容器（Tomcat 等），被映射到 `DispatcherServlet`。
3. `DispatcherServlet` 通过 `HandlerMapping` 找到对应的 Handler（Controller 方法）。
4. 通过 `HandlerAdapter` 调用 Handler：完成参数解析、数据绑定、校验。
5. Handler 返回 `ModelAndView` 或对象结果。
6. `DispatcherServlet` 调用 `ViewResolver` 解析视图并渲染，或通过 `HttpMessageConverter` 输出 JSON/XML。
7. 响应返回客户端。

#### 2.1.1 简化版 doDispatch 伪代码

```java
protected void doDispatch(HttpServletRequest request, HttpServletResponse response) {
    // 1. 查找 Handler
    HandlerExecutionChain mappedHandler = getHandler(request);
    // 2. 查找合适的 HandlerAdapter
    HandlerAdapter ha = getHandlerAdapter(mappedHandler.getHandler());
    // 3. 执行拦截器 preHandle
    mappedHandler.applyPreHandle(request, response);
    // 4. 真正调用 Controller 方法
    ModelAndView mv = ha.handle(request, response, mappedHandler.getHandler());
    // 5. 执行拦截器 postHandle
    mappedHandler.applyPostHandle(request, response, mv);
    // 6. 视图渲染
    render(mv, request, response);
}
```

### 2.2 DispatcherServlet：前端控制器的模板方法

- **本质**：继承自 `FrameworkServlet`，内部使用模板方法固定了请求处理主流程：
  - `doService()` -> `doDispatch()`。
- **关键步骤**（简化）：
  - 获取、缓存 HandlerMapping / HandlerAdapter。
  - 调用拦截器 `preHandle`。
  - 调用目标 Controller。
  - 调用拦截器 `postHandle`。
  - 处理异常（交给 `HandlerExceptionResolver`）。
  - 渲染视图。

### 2.3 HandlerMapping：策略模式的体现

- **职责**：根据请求信息找到“哪个方法”来处理该请求。
- **常见实现**：
  - `RequestMappingHandlerMapping`：基于 `@RequestMapping` 等注解。
- **设计特点**：
  - 使用接口 `HandlerMapping` + 多个实现类，符合开放封闭原则。
  - 你可以自定义一个 HandlerMapping，实现特殊路由规则。

### 2.4 HandlerAdapter：适配不同类型的 Handler

- **问题**：项目中可能有不同风格的 Controller（基于注解、基于接口等）。
- **作用**：
  - 屏蔽 Handler 具体类型差异，对上层（DispatcherServlet）暴露统一的 `handle()` 调用方式。
- **常见实现**：
  - `RequestMappingHandlerAdapter`：配合注解型 Controller。

### 2.5 数据绑定与参数解析

- **WebDataBinder**：负责将请求参数（字符串）转换成方法参数所需的 Java 类型。
- **HandlerMethodArgumentResolver**：
  - 按参数类型或注解（如 `@RequestParam`、`@PathVariable`、`@RequestBody`）决定如何从请求中取值。
  - 是一个扩展点：可以自定义解析器支持特殊参数类型（例如当前登录用户对象）。

#### 2.5.1 自定义参数解析器示例（解析当前用户）

```java
public class CurrentUserArgumentResolver implements HandlerMethodArgumentResolver {

    @Override
    public boolean supportsParameter(MethodParameter parameter) {
        return parameter.getParameterType().equals(CurrentUser.class);
    }

    @Override
    public Object resolveArgument(MethodParameter parameter,
                                  ModelAndViewContainer mavContainer,
                                  NativeWebRequest webRequest,
                                  WebDataBinderFactory binderFactory) {
        // 从请求头或 Token 中解析当前用户
        String userId = webRequest.getHeader("X-User-Id");
        return new CurrentUser(userId);
    }
}
```

### 2.6 返回值处理与视图解析

- **HandlerMethodReturnValueHandler**：
  - 根据返回值类型和注解决定如何处理：视图名、重定向、JSON 等。
- **ViewResolver**：
  - 负责逻辑视图名 -> 物理视图的映射。
  - 例如：`InternalResourceViewResolver` 将 `"user/list"` 映射到 `/WEB-INF/views/user/list.jsp`。
- **HttpMessageConverter**：
  - 针对 `@ResponseBody` 或 `RestController`，将对象转换为 JSON、XML 等。

### 2.7 拦截器链与 AOP 的协同

- **HandlerInterceptor**：在 Controller 调用前后、视图渲染前后切入，适合做：
  - 登录校验、权限检查、日志记录、性能监控等。

#### 2.7.1 拦截器示例：简单的日志拦截器

```java
public class LogInterceptor implements HandlerInterceptor {

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) {
        System.out.println("request start: " + request.getRequestURI());
        return true;
    }

    @Override
    public void afterCompletion(HttpServletRequest request, HttpServletResponse response, Object handler, Exception ex) {
        System.out.println("request end: " + request.getRequestURI());
    }
}
```

- **AOP**：可针对 Service 层做事务、缓存、审计等，与 Web 拦截器配合形成完整的横切方案。

### 2.8 从“能用”到“会用”的关键

- **掌握主流程**：清楚一个请求从进入到响应大致经历哪些组件。
- **掌握扩展点**：HandlerMapping、HandlerAdapter、ArgumentResolver、ReturnValueHandler、ViewResolver、Interceptor。
- **实践建议**：
  - 在学习过程中尝试**自定义一个简单的 HandlerMethodArgumentResolver 和 Interceptor**，体会扩展机制。

### 2.1 整体请求处理流程（高层视角）

1. 客户端发起 HTTP 请求。
2. 请求到达 Servlet 容器（Tomcat 等），被映射到 `DispatcherServlet`。
3. `DispatcherServlet` 通过 `HandlerMapping` 找到对应的 Handler（Controller 方法）。
4. 通过 `HandlerAdapter` 调用 Handler：完成参数解析、数据绑定、校验。
5. Handler 返回 `ModelAndView` 或对象结果。
6. `DispatcherServlet` 调用 `ViewResolver` 解析视图并渲染，或通过 `HttpMessageConverter` 输出 JSON/XML。
7. 响应返回客户端。

### 2.2 DispatcherServlet：前端控制器的模板方法

- **本质**：继承自 `FrameworkServlet`，内部使用模板方法固定了请求处理主流程：
  - `doService()` -> `doDispatch()`。
- **关键步骤**（简化）：
  - 获取、缓存 HandlerMapping / HandlerAdapter。
  - 调用拦截器 `preHandle`。
  - 调用目标 Controller。
  - 调用拦截器 `postHandle`。
  - 处理异常（交给 `HandlerExceptionResolver`）。
  - 渲染视图。

### 2.3 HandlerMapping：策略模式的体现

- **职责**：根据请求信息找到“哪个方法”来处理该请求。
- **常见实现**：
  - `RequestMappingHandlerMapping`：基于 `@RequestMapping` 等注解。
- **设计特点**：
  - 使用接口 `HandlerMapping` + 多个实现类，符合开放封闭原则。
  - 你可以自定义一个 HandlerMapping，实现特殊路由规则。

### 2.4 HandlerAdapter：适配不同类型的 Handler

- **问题**：项目中可能有不同风格的 Controller（基于注解、基于接口等）。
- **作用**：
  - 屏蔽 Handler 具体类型差异，对上层（DispatcherServlet）暴露统一的 `handle()` 调用方式。
- **常见实现**：
  - `RequestMappingHandlerAdapter`：配合注解型 Controller。

### 2.5 数据绑定与参数解析

- **WebDataBinder**：负责将请求参数（字符串）转换成方法参数所需的 Java 类型。
- **HandlerMethodArgumentResolver**：
  - 按参数类型或注解（如 `@RequestParam`、`@PathVariable`、`@RequestBody`）决定如何从请求中取值。
  - 是一个扩展点：可以自定义解析器支持特殊参数类型（例如当前登录用户对象）。

### 2.6 返回值处理与视图解析

- **HandlerMethodReturnValueHandler**：
  - 根据返回值类型和注解决定如何处理：视图名、重定向、JSON 等。
- **ViewResolver**：
  - 负责逻辑视图名 -> 物理视图的映射。
  - 例如：`InternalResourceViewResolver` 将 `"user/list"` 映射到 `/WEB-INF/views/user/list.jsp`。
- **HttpMessageConverter**：
  - 针对 `@ResponseBody` 或 `RestController`，将对象转换为 JSON、XML 等。

### 2.7 拦截器链与 AOP 的协同

- **HandlerInterceptor**：在 Controller 调用前后、视图渲染前后切入，适合做：
  - 登录校验、权限检查、日志记录、性能监控等。
- **AOP**：可针对 Service 层做事务、缓存、审计等，与 Web 拦截器配合形成完整的横切方案。

### 2.8 从“能用”到“会用”的关键

- **掌握主流程**：清楚一个请求从进入到响应大致经历哪些组件。
- **掌握扩展点**：HandlerMapping、HandlerAdapter、ArgumentResolver、ReturnValueHandler、ViewResolver、Interceptor。
- **实践建议**：
  - 在学习过程中尝试**自定义一个简单的 HandlerMethodArgumentResolver 和 Interceptor**，体会扩展机制。
