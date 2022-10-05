# vendor_hpmicro

## 介绍

该仓库托管HPMicro开发的基于hpm6750evk2产品样例代码。

## 产品样例

支持基于hpm6750 SoC的hpm6750evk2单板进行开发的应用、XTS测试样例。

代码路径：

```
device/                               --- 硬件单板相关仓库
├── board/hpmicro                     --- HPMicro相关单板
├── soc/hpmicro                       --- HPMicro相关SoC代码
vendor/hpmicro/                       --- vendor_goodix 仓库路径
└── hpm6750evk2                       --- 样例
```

## 编译XTS测试

执行`hb build -f --gn-args="build_xts=true"`命令进行xts编译。

注意:编译xts需要需要打如下补丁，修改线程栈为1800，否则会出现栈溢出崩溃

```bash
diff --git a/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_singletask_func_test.c b/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_singletask_func_test.c
index 7fc006e18..dd66409e9 100755
--- a/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_singletask_func_test.c
+++ b/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_singletask_func_test.c
@@ -201,7 +201,7 @@ static TaskConfig GetTaskConfig(Service *service)
     // queueSize: [0, system upper limit), 0: will not create taskpool, the max value depends on RAM size
     // priority: PRI_ABOVE_NORMAL PRI_NORMAL PRI_BELOW_NORMAL PRI_LOW
 
-    TaskConfig config = { LEVEL_HIGH, PRI_NORMAL, 1600, 2, SINGLE_TASK };
+    TaskConfig config = { LEVEL_HIGH, PRI_NORMAL, 1800, 2, SINGLE_TASK };
     if (service == (Service *)&g_service[INDEX0]) {
         config.priority = PRI_LOW;
     } else if (service == (Service *)&g_service[INDEX1]) {
@@ -571,4 +571,4 @@ LITE_TEST_CASE(SingleTaskFuncTestSuite, testSingleTask0050, Function | MediumTes
     }
 };
 
diff --git a/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_specifiedtask_func_test.c b/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_specifiedtask_func_test.c
index 734a22081..4b5b65562 100755
--- a/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_specifiedtask_func_test.c
+++ b/distributed_schedule_lite/system_ability_manager_hal/src/taskpool_specifiedtask_func_test.c
@@ -148,7 +148,7 @@ static BOOL MessageHandle(Service *service, Request *msg)
 static TaskConfig GetTaskConfig(Service *service)
 {
     (void)service;
-    TaskConfig config = {LEVEL_HIGH, PRI_NORMAL, 1600, 20, SPECIFIED_TASK};
+    TaskConfig config = {LEVEL_HIGH, PRI_NORMAL, 1800, 20, SPECIFIED_TASK};
     return config;
 }
 
@@ -400,4 +400,4 @@ LITE_TEST_CASE(SpecifiedTaskFuncTestSuite, testSpecifiedTask0030, Function | Med
     osDelay(OPER_INTERVAL);
     TEST_ASSERT_EQUAL_INT(g_servicePoint1 == g_servicePoint2, TRUE);
 };

```

## 相关仓库

[vendor_hpmicro](https://gitee.com/openharmony-sig/vendor_hpmicro)

[device_soc_hpmicro](https://gitee.com/openharmony-sig/device_soc_hpmicro)

[device_board_hpmicro](https://gitee.com/openharmony-sig/device_board_hpmicro)

## 联系

如果您在开发过程中有问题，请在仓库[issues](https://gitee.com/openharmony-sig/vendor_hpmicro/issues)提问。
