# SpringMVC 学习路径总览

- **目标读者**：已理解 Java 基础语法与常见设计模式（如单例、工厂、策略、模板方法、观察者等）的开发者。
- **学习主线**：从设计思想 -> 框架机制 -> 基本使用 -> 典型业务落地 -> 综合高级场景。
- **阅读顺序建议**：
  1. `01_SpringMVC_Design_Overview.md`
  2. `02_SpringMVC_Core_Mechanism.md`
  3. `03_SpringMVC_Basic_Usage.md`
  4. `04_SpringMVC_Business_Application.md`
  5. `05_SpringMVC_Advanced_Application.md`

---

## 文档列表

1. **`01_SpringMVC_Design_Overview.md`**

   - 从设计模式角度理解 Web MVC
   - SpringMVC 在整体架构中的位置
   - 核心参与者和职责划分

2. **`02_SpringMVC_Core_Mechanism.md`**

   - DispatcherServlet 作为前端控制器
   - HandlerMapping / HandlerAdapter / ViewResolver 等核心扩展点
   - 常见设计模式在 SpringMVC 中的体现

3. **`03_SpringMVC_Basic_Usage.md`**

   - 基本项目结构
   - Controller / RequestMapping / 参数绑定 / 返回值处理
   - 表单处理与简单校验

4. **`04_SpringMVC_Business_Application.md`**

   - 典型三层架构：Controller-Service-Repository
   - RESTful API 设计与异常处理
   - 统一返回体与统一错误码设计

5. **`05_SpringMVC_Advanced_Application.md`**
   - 拦截器、过滤器、AOP 在 Web 层的协同
   - 统一日志、审计、限流等横切关注点
   - 模块化与可扩展性设计
