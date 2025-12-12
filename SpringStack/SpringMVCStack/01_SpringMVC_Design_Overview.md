## 01：从设计模式看 SpringMVC 设计思想

> 本篇以**原生 SpringMVC（基于 Servlet 的基础框架）**为背景，通过对比“自己手写一个迷你 MVC”来理解 SpringMVC 的设计思想。

### 1.1 传统 Web 开发痛点与 MVC 出现的背景

- **Servlet 直连模式的问题**：
  - **强耦合**：一个 Servlet 同时处理请求解析、业务逻辑、视图跳转、异常处理。
  - **难以测试**：所有逻辑都依赖 `HttpServletRequest/Response`。
  - **难以扩展**：横切需求（日志、权限、统一异常）混杂在业务代码中。
- **MVC 模式的目标**：
  - **M（Model）**：领域模型与业务状态。
  - **V（View）**：负责展示，不包含业务逻辑。
  - **C（Controller）**：负责请求协调与流程编排，不做业务计算。

#### 1.1.1 代码对比：原始 Servlet 写法

```java
// 一个典型的 Servlet 写法：所有逻辑都堆在一起
public class UserServlet extends HttpServlet {
    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
        String action = req.getParameter("action");
        if ("detail".equals(action)) {
            // 解析参数
            String id = req.getParameter("id");
            // 业务逻辑（一般会直接 new DAO）
            User user = findUserById(id);
            // 视图输出（直接拼接 HTML / JSON）
            resp.setContentType("application/json;charset=UTF-8");
            resp.getWriter().write(toJson(user));
        }
        // 其他 action ...
    }
}
```

### 1.2 Java 常见设计模式在 Web MVC 中的角色

- **前端控制器模式（Front Controller）**：统一入口拦截所有请求，集中处理公共逻辑。
- **策略模式（Strategy）**：不同请求使用不同的处理策略，如不同的 Handler、不同的视图解析方式。
- **模板方法模式（Template Method）**：在抽象流程中固定调用顺序，允许子类或扩展点定制某些步骤。
- **适配器模式（Adapter）**：封装不同风格的 Controller，使框架可以统一调用。
- **观察者模式（Observer）**：事件机制，如请求事件、上下文刷新事件。

#### 1.2.1 手写一个极简 Front Controller 示例

```java
// 一个极简的“前端控制器”，只处理 REST 风格 JSON
public class MiniDispatcherServlet extends HttpServlet {

    // 简化：用 Map 模拟 URL 到 Handler 的映射
    private final Map<String, Controller> handlerMapping = new HashMap<>();

    @Override
    public void init() {
        // 初始化阶段注册 Handler（实际 SpringMVC 会自动扫描注解）
        handlerMapping.put("/users", new UserListController());
        handlerMapping.put("/users/detail", new UserDetailController());
    }

    @Override
    protected void service(HttpServletRequest req, HttpServletResponse resp) throws IOException {
        String path = req.getRequestURI();
        Controller controller = handlerMapping.get(path);
        if (controller == null) {
            resp.setStatus(HttpServletResponse.SC_NOT_FOUND);
            return;
        }
        // 模板方法：固定调用流程
        Object result = controller.handle(req);
        writeJson(resp, result);
    }

    private void writeJson(HttpServletResponse resp, Object result) throws IOException {
        resp.setContentType("application/json;charset=UTF-8");
        resp.getWriter().write(toJson(result));
    }

    interface Controller {
        Object handle(HttpServletRequest request);
    }
}
```

### 1.3 Spring 全家桶中的 Web 层定位

- **Spring Core & Beans**：IoC 容器与 Bean 生命周期管理。
- **Spring MVC**：基于 Servlet 的 Web MVC 框架，负责 HTTP 请求到业务逻辑的映射。
- **与 Spring Boot 的关系**：Boot 负责自动配置与约定优于配置，底层 Web 仍可基于 SpringMVC。

### 1.4 SpringMVC 架构总览（从设计角度看组件分工）

- **DispatcherServlet**：
  - 前端控制器，统一入口。
  - 负责**请求分发、异常捕获、视图渲染流程控制**。
- **HandlerMapping**：
  - 根据 URL、HTTP 方法、注解信息等**查找具体 Handler（Controller 方法）**。
  - 是策略模式的典型应用：可插拔多种映射策略。
- **HandlerAdapter**：
  - 负责调用具体 Handler，使框架与 Controller 解耦。
  - 使用适配器模式屏蔽不同 Controller 写法的差异。
- **HandlerExceptionResolver**：
  - 统一异常捕获与转译，输出为视图或 JSON 结果。
- **ViewResolver / View**：
  - 将逻辑视图名解析为具体视图实现（JSP、Thymeleaf、JSON 等）。

### 1.5 SpringMVC 的设计目标（从框架设计者视角）

- **解耦**：控制层与 Servlet API、视图技术、URL 映射算法解耦。
- **可扩展**：通过接口 + 模板方法 + 策略模式预留扩展点。
- **易测试**：Controller 方法可以被普通单元测试直接调用。
- **与 Spring 容器深度集成**：充分利用 IoC、AOP 等能力。

### 1.6 学习建议

- **先从“问题”出发**：思考如果只用原生 Servlet，你会如何设计 URL 映射、异常处理、权限校验？
- **再对照 SpringMVC 提供的组件**：分析它如何用设计模式系统化解决这些问题。
- **最后在阅读源码或文档时，有意识地标记设计模式的使用位置。**

### 1.1 传统 Web 开发痛点与 MVC 出现的背景

- **Servlet 直连模式的问题**：
  - **强耦合**：一个 Servlet 同时处理请求解析、业务逻辑、视图跳转、异常处理。
  - **难以测试**：所有逻辑都依赖 `HttpServletRequest/Response`。
  - **难以扩展**：横切需求（日志、权限、统一异常）混杂在业务代码中。
- **MVC 模式的目标**：
  - **M（Model）**：领域模型与业务状态。
  - **V（View）**：负责展示，不包含业务逻辑。
  - **C（Controller）**：负责请求协调与流程编排，不做业务计算。

### 1.2 Java 常见设计模式在 Web MVC 中的角色

- **前端控制器模式（Front Controller）**：统一入口拦截所有请求，集中处理公共逻辑。
- **策略模式（Strategy）**：不同请求使用不同的处理策略，如不同的 Handler、不同的视图解析方式。
- **模板方法模式（Template Method）**：在抽象流程中固定调用顺序，允许子类或扩展点定制某些步骤。
- **适配器模式（Adapter）**：封装不同风格的 Controller，使框架可以统一调用。
- **观察者模式（Observer）**：事件机制，如请求事件、上下文刷新事件。

### 1.3 Spring 全家桶中的 Web 层定位

- **Spring Core & Beans**：IoC 容器与 Bean 生命周期管理。
- **Spring MVC**：基于 Servlet 的 Web MVC 框架，负责 HTTP 请求到业务逻辑的映射。
- **与 Spring Boot 的关系**：Boot 负责自动配置与约定优于配置，底层 Web 仍可基于 SpringMVC。

### 1.4 SpringMVC 架构总览（从设计角度看组件分工）

- **DispatcherServlet**：
  - 前端控制器，统一入口。
  - 负责**请求分发、异常捕获、视图渲染流程控制**。
- **HandlerMapping**：
  - 根据 URL、HTTP 方法、注解信息等**查找具体 Handler（Controller 方法）**。
  - 是策略模式的典型应用：可插拔多种映射策略。
- **HandlerAdapter**：
  - 负责调用具体 Handler，使框架与 Controller 解耦。
  - 使用适配器模式屏蔽不同 Controller 写法的差异。
- **HandlerExceptionResolver**：
  - 统一异常捕获与转译，输出为视图或 JSON 结果。
- **ViewResolver / View**：
  - 将逻辑视图名解析为具体视图实现（JSP、Thymeleaf、JSON 等）。

### 1.5 SpringMVC 的设计目标（从框架设计者视角）

- **解耦**：控制层与 Servlet API、视图技术、URL 映射算法解耦。
- **可扩展**：通过接口 + 模板方法 + 策略模式预留扩展点。
- **易测试**：Controller 方法可以被普通单元测试直接调用。
- **与 Spring 容器深度集成**：充分利用 IoC、AOP 等能力。

### 1.6 学习建议

- **先从“问题”出发**：思考如果只用原生 Servlet，你会如何设计 URL 映射、异常处理、权限校验？
- **再对照 SpringMVC 提供的组件**：分析它如何用设计模式系统化解决这些问题。
- **最后在阅读源码或文档时，有意识地标记设计模式的使用位置。**
