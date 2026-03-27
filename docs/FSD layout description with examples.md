# Functional Specification Document

## 1. Overview

### 1.1 Purpose

- Business problem or goal  
- Why this system or feature exists  
- Expected business value  

**Example:**  
Enable remote monitoring of IoT devices to reduce manual maintenance effort.



### 1.2 Scope

- What is included  
- What is explicitly excluded  
- Functional boundaries of the system  

**Example:**  
Includes device status monitoring and alerting. Excludes firmware updates and device provisioning.



### 1.3 Users / Roles

- User groups interacting with the system  
- Their responsibilities and level of access  

**Example:**  
- Admin: configures the system and manages users  
- Operator: monitors device status and handles alerts  
- Viewer: has read-only access  



## 2. Functional Summary

### 2.1 Main Workflow / Operating Modes

- Main system workflow  
- Main operational states or modes  
- High-level process overview  

**Example:**  
- Normal mode: system collects and displays device data  
- Debug mode: system outputs additional diagnostic information  



### 2.2 System Architecture

- High-level system structure  
- Main software and hardware components  
- Component interactions and system boundaries  

**Example:**  
```
┌──────────────────────────────────────────────────────────────────────────┐
│                          Network (192.168.0.x)                           │
└──────────────────────────────────────────────────────────────────────────┘
       │  eth0 (USB Ethernet)                      │
       │                                           │
       ▼                                           ▼
┌────────────────────────┐              ┌────────────────────┐
│  VM Host               │              │  IoT Device        │
│                        │              └────────────────────┘
│  ┌──────────────────┐  │
│  │ Container A      │  │
│  │ - Web Server     │  │  
│  │ - Backend API    │  │  
│  └──────────────────┘  │  
│  ┌──────────────────┐  │
│  │ Container B      │  │
│  │ - DB             │  │  
│  └──────────────────┘  │  
│  ┌──────────────────┐  │
│  │ Container C      │  │
│  │ - Device Monitor │  │  
│  └──────────────────┘  │  
└────────────────────────┘
```


### 2.3 Hardware Description

- Dedicated computing hardware  
- External devices  
- Relevant hardware interfaces  
- System boundaries, if applicable  

**Example:**  
- Server: Linux VM  
- Devices: ESP32 sensors  
- Network: Ethernet and Wi-Fi communication  



### 2.4 Key Components

- Core software and hardware components  
- Purpose of each major component  

**Example:**  
- Web Server: serves the user interface  
- Database: stores device data and system state  
- Backend API: provides REST endpoints for the web application  
- Device Monitor: monitors IoT devices and updates the database  



## 3. Definitions (optional)

### 3.1 Terms and Abbreviations

- Domain-specific terms  
- Abbreviations used throughout the document  

**Example:**

| Name   | Description                                  |
|--|-|
| Device | IoT HW device sending data                   |
| Alert  | Notification triggered by a threshold breach |



## 4. Functional Requirements

- List of functional requirements  
- One requirement per subsection  
- Title format: `<requirement name> (FR-xxx)`  

Each functional requirement should contain:
- Description: What the system must do  
- Trigger: What starts the action  
- Behavior: Expected system behavior  
- Expected Result: Clear, testable outcome  

**Example:**

### 4.1 Status Overview (FR-001)

Description:  
The system displays the current status of all devices.

Trigger:  
The user opens the dashboard.

Behavior:
- The system retrieves device data  
- The system displays the status of each device  
- Status values include online, offline, and error  

Expected Result:
- All devices are listed  
- Status updates are visible within 5 seconds  
- Offline devices are clearly marked  



## 5. Non-Functional Requirements

- List of non-functional requirements  
- One requirement per subsection  
- Focus on quality attributes and operational constraints (e.g. performance, reliability, security, maintainability, determinism)

**Example:**

### 5.1 Determinism (NFR-001)

Description:  
The system behaves predictably across restarts and deployments.

Expected Result:
- Configuration survives reboots  
- No dynamic IP assignment is used  



## 6. Data

- Main data flow  
- Data sources and destinations  
- Data formats  
- Validation rules  
- Storage or persistence, if relevant  

**Example:**  
- Source: IoT devices via MQTT  
- Format: JSON {device_id, temperature, timestamp}  
- Validation: temperature must be within -40 to 125°C  
- Storage: validated data is stored in the database  



## 7. User Interface

- UI structure and behavior  
- Screens, views, and interactions  
- One subsection per page or major view  

**Example:**

### 7.1 Dashboard

Overview page showing all devices.
- device list  
- current device status  
- active alerts  
- quick navigation to detail views  



## 8. External Dependencies / Integrations

- External systems or services  
- Interfaces and exchanged data  
- High-level integration behavior  

**Example:**

### 8.1 Email Service

The system is connected to an external mail server for notifications.
- Mail server: example.com  
- Protocol: SMTP  
- Usage: alert notifications  



## 9. Permissions (if relevant)

- Access control rules  
- Who can view, create, edit, delete, approve, or administer  

**Example:**  
- Admin: full access  
- Operator: read access and alert acknowledgement  
- Viewer: read-only access  



## 10. Test Cases

- Key scenarios for validation  
- One test case per subsection  
- Title format: `<test case name> (TC-xxx)`  

Each test case should contain:
- Steps: What actions are performed  
- Related Requirements: Linked requirement IDs  
- Pass Criteria: Clear yes/no criteria  

**Example:**

### 10.1 Device Registration (TC-001)

Steps:
1. A new IoT device is added to the network  
2. The device registers itself with the system  

Pass Criteria:
- A new entry for the device is created in the database  

Related Requirements:
- FR-001  



## 11. Open Points / Assumptions

### 11.1 Open Questions

- Unclear or unresolved points  
- Issues that still need a decision  

**Example:**  
- Should alerts be persisted after restart?  



### 11.2 Assumptions

- Assumptions about external systems or expected behavior  

**Example:**  
- Devices send data every 10 seconds  
- Network connectivity is available during normal operation  



## Appendix A: Technical Details

## Appendix B: Implementation Plan

### B.1 Implementation Plan

- Implementation steps and planned tasks  

**Example:**  
- [x] Implement device ingestion service  
- [ ] Build dashboard UI  
- [ ] Add alert notification workflow  