## 05：SpringMVC 综合高级应用与架构思考

> 本篇开始从“单体中的好实践”过渡到“面向微服务的 Web 层设计”，重点放在：拦截器/过滤器/AOP 的分工、自定义扩展点、以及与领域设计/微服务拆分的衔接。

### 5.1 拦截器、过滤器与 AOP 的协同设计

- **Filter（Servlet 过滤器）**：
  - 工作在 Servlet 容器层，与具体框架无关，适合做：编码过滤、跨域、基础安全过滤等。
- **HandlerInterceptor（SpringMVC 拦截器）**：
  - 工作在 Handler 调用前后，可访问 Handler 信息，适合做：认证鉴权、接口级日志、性能统计等。
- **AOP（切面）**：
  - 一般作用在 Service 层，适合做：事务控制、缓存、审计日志等。
- **设计要点**：
  - 明确每一层处理的“关注点范围”，避免重复与混乱。

#### 5.1.1 典型权限拦截器示例

```java
public class AuthInterceptor implements HandlerInterceptor {

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {
        String token = request.getHeader("Authorization");
        if (token == null || !checkToken(token)) {
            response.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
            return false;
        }
        return true;
    }
}
```

### 5.2 自定义扩展点：让框架适应你的业务

- **自定义 HandlerMethodArgumentResolver**：
  - 将“当前登录用户”等常用对象自动注入到 Controller 方法参数中。
- **自定义 HandlerMethodReturnValueHandler**：
  - 实现统一响应包装（如自动将返回对象包装成 `ApiResponse`）。
- **自定义异常解析器 HandlerExceptionResolver**：
  - 实现复杂业务异常到 HTTP 响应的映射规则。

#### 5.2.1 统一响应包装的自定义 ReturnValueHandler 简化示例

```java
public class ApiResponseReturnValueHandler implements HandlerMethodReturnValueHandler {

    @Override
    public boolean supportsReturnType(MethodParameter returnType) {
        return !ApiResponse.class.isAssignableFrom(returnType.getParameterType());
    }

    @Override
    public void handleReturnValue(Object returnValue,
                                  MethodParameter returnType,
                                  ModelAndViewContainer mavContainer,
                                  NativeWebRequest webRequest) throws Exception {
        mavContainer.setRequestHandled(true);
        ApiResponse<Object> body = ApiResponse.success(returnValue);
        HttpServletResponse response = webRequest.getNativeResponse(HttpServletResponse.class);
        writeJson(response, body);
    }
}
```

### 5.3 领域驱动设计（DDD）与 SpringMVC 的结合（简要）

- **分工思路**：
  - Controller 作为“应用服务”的 HTTP 外观。
  - Service 内部再进一步拆分为应用服务与领域服务。
  - Repository 更关注领域对象的持久化。
- **好处**：
  - 更好地应对复杂业务与长期演进。

### 5.4 大型系统中的模块化与边界

- **按业务域划分模块**：
  - 如 `user-service`, `order-service` 等子工程或模块。
- **API 网关与后端服务**：
  - SpringMVC 控制的仅是单个服务的 Web 层。
  - 对外可以通过网关做统一路由、限流、灰度发布等。

### 5.5 性能与可用性考虑

- **缓存策略**：页面缓存、接口级缓存、热点数据缓存。
- **限流与熔断**：与拦截器、网关、熔断组件（如 Resilience4j）配合。
- **异步与消息化**：对耗时操作进行异步化或通过消息队列解耦。

### 5.6 学习与实践路径建议

- **阶段 1：理解机制**
  - 梳理 DispatcherServlet 的主流程，理解每个扩展点的职责。
- **阶段 2：完成一个“规范的”中小型业务模块**
  - 实现用户/订单等模块，做到 URL 设计规范、异常与日志统一。
- **阶段 3：尝试高级扩展**
  - 自定义参数解析、统一响应封装、复杂权限控制。
- **阶段 4：结合 DDD / 微服务架构思考整体设计**
  - 在更大范围内看待 SpringMVC 所在的层次与职责。
