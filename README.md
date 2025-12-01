# Система управления компанией

## Описание задания

Данное приложение представляет собой систему управления компанией с графическим интерфейсом на базе Qt6. Система позволяет управлять сотрудниками, проектами и задачами, а также автоматически назначать сотрудников на задачи с учетом их квалификации, загрузки и бюджета проекта.

### Основной функционал:

- **Управление компаниями**: создание и управление несколькими компаниями с информацией о названии, отрасли, местоположении и годе основания
- **Управление сотрудниками**: 
  - Добавление, редактирование и удаление сотрудников
  - Поддержка различных типов сотрудников: Manager, Developer, Designer, QA
  - Управление статусом сотрудников (активный/неактивный)
  - Отслеживание загрузки сотрудников (часы в неделю)
  - История проектов сотрудников
- **Управление проектами**:
  - Создание и редактирование проектов с различными фазами разработки (Planning, Development, Testing, Deployment, Completed)
  - Управление задачами в рамках проектов
  - Расчет стоимости проектов
  - Автоматическое назначение сотрудников на проекты
- **Назначение задач**:
  - Ручное и автоматическое назначение сотрудников на задачи
  - Валидация назначений с учетом роли сотрудника, типа задачи и фазы проекта
  - Проверка доступности сотрудников и бюджетных ограничений
- **Статистика**: визуализация данных о компании, сотрудниках и проектах
- **Автосохранение**: автоматическое сохранение и загрузка данных при запуске/закрытии приложения

## Инструкция по запуску

### Требования

- **CMake** версии 3.28 или выше
- **Qt6** (Core, Widgets)
- **C++ компилятор** с поддержкой стандарта C++23

### Сборка проекта

1. Клонируйте репозиторий:
```bash
git clone https://github.com/ekuzm/course-work.git
cd course-work
```

2. Создайте директорию для сборки:
```bash
mkdir build
cd build
```

3. Настройте проект с помощью CMake:
```bash
cmake ..
```

4. Соберите проект:
```bash
cmake --build .
```

5. Запустите приложение:
```bash
./course-work
```

### Структура данных

Приложение автоматически создает директорию `build/data/` для хранения данных:
- `companies/` - информация о компаниях
- `employees/` - данные о сотрудниках
- `projects/` - информация о проектах и задачах

## SonarCloud

Анализ качества кода доступен на SonarCloud:

🔗 [Перейти к анализу на SonarCloud]((https://sonarcloud.io/project/overview?id=ekuzm_course-work))

## Структура проекта

```
course-work/
├── CMakeLists.txt          
├── include/                
│   ├── entities/          
│   │   ├── company.h      
│   │   ├── employee.h     
│   │   ├── derived_employees.h  
│   │   ├── project.h      
│   │   ├── task.h         
│   │   ├── company_managers.h    
│   │   └── company_task_operations.h
│   ├── exceptions/        
│   │   ├── exceptions.h
│   │   └── exception_handler.h
│   ├── helpers/           
│   │   ├── dialog_helper.h
│   │   ├── display_helper.h
│   │   ├── employee_dialog_helper.h
│   │   ├── project_dialog_helper.h
│   │   ├── task_dialog_helper.h
│   │   ├── validation_helper.h
│   │   └── ...
│   ├── managers/          
│   │   ├── company_manager.h
│   │   ├── file_manager.h
│   │   └── auto_save_loader.h
│   ├── services/          
│   │   ├── cost_calculation_service.h
│   │   ├── task_assignment_service.h
│   │   ├── employee_service.h
│   │   └── project_service.h
│   ├── ui/                
│   │   ├── main_window.h
│   │   ├── main_window_operations.h
│   │   ├── main_window_helpers.h
│   │   ├── main_window_ui_builder.h
│   │   └── statistics_chart_widget.h
│   └── utils/             
│       ├── consts.h
│       ├── app_styles.h
│       └── container_utils.h
├── src/                   
│   ├── main.cpp          
│   ├── entities/
│   ├── exceptions/
│   ├── helpers/
│   ├── managers/
│   ├── services/
│   ├── ui/
│   └── utils/
└── build/                 
    └── data/             
```


