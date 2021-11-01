# Riru - 改机模块

利用 [Riru](https://github.com/RikkaApps/Riru) 模块实现改机的模块.

## 使用方法

- 安装Magisk和riru模块
- 下载[本模块](https://github.com/luoyesiqiu/Riru-gaiji/releases/tag/v1.0.0)
- 将本模块推入手机
- 在MagiskManager中刷入本模块
- 重启手机
- 根据实际需求修改build-conf.prop和prop-prop，并将它们推入手机的·`/data/local/tmp/`目录

### build-conf.prop

该文件对应android.os.Build类下的字段，比如想修改机型为`Mi5`则内容应为：

```
MODEL=Mi5
```

### prop-conf.prop

该文件对应System Property的字段，比如想修改机型为`Mi5`则内容应为：

```
ro.product.model=Mi5
```