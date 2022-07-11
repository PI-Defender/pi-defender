# Introduction

PI-Defender is a kernel security driver used to block past, current and future process injection techniques on Windows operating system:
* [Process Hollowing](https://attack.mitre.org/techniques/T1055/012/)
* [Process Doppelgänging](https://attack.mitre.org/techniques/T1055/013/)
* [Process Herpaderping](https://jxy-s.github.io/herpaderping/)
* [Process Ghosting](https://www.elasstic.co/fr/blog/process-ghosting-a-new-executable-image-tampering-attack)
* [Process Overwritting](https://github.com/hasherazade/process-overwriting)
* [Dll Injection](https://attack.mitre.org/techniques/T1055/001/)

## How does it works ?

Whenever a binary wants to interact with the memory of a Windows Object, it has to specify rights according to what it wants to achieve.  
Then, the system returns a *handle* associated to this object with the granted rights.  
Since all process injection techniques need to write in the memory of a remote process, they need a handle with specific rights (PROCESS_VM_WRITE and PROCESS_VM_OPERATION).  
The technique used by PI-Defender is simply to **remove forbidden access rights on handles associated with remote processes**.

### Driver Kernel

#### Handle Rights

| Requested                         | Granted                   |
| ---------------------------------	| :-----------------------: |
| DELETE                            | \emoji :heavy_check_mark: |
| READ_CONTROL                      | \emoji :heavy_check_mark: |
| WRITE_DAC                         | \emoji :heavy_check_mark: |
| WRITE_OWNER                       | \emoji :heavy_check_mark: |
| SYNCHRONIZE                       | \emoji :heavy_check_mark: |
| PROCESS_TERMINATE                 | \emoji :heavy_check_mark: |
| PROCESS_CREATE_THREAD             | \emoji :heavy_check_mark: |
| PROCESS_SET_SESSIONID             | \emoji :heavy_check_mark: |
| PROCESS_VM_OPERATION              | \emoji :x:                |
| PROCESS_VM_READ                   | \emoji :heavy_check_mark: |
| PROCESS_VM_WRITE                  | \emoji :x:                |
| PROCESS_DUP_HANDLE                | \emoji :heavy_check_mark: |
| PROCESS_CREATE_PROCESS            | \emoji :heavy_check_mark: |
| PROCESS_SET_QUOTA                 | \emoji :heavy_check_mark: |
| PROCESS_SET_INFORMATION           | \emoji :heavy_check_mark: |
| PROCESS_QUERY_INFORMATION         | \emoji :heavy_check_mark: |
| PROCESS_SUSPEND_RESUME            | \emoji :heavy_check_mark: |
| PROCESS_QUERY_LIMITED_INFORMATION | \emoji :heavy_check_mark: |
| PROCESS_SET_LIMITED_INFORMATION   | \emoji :heavy_check_mark: |

For detailled information about Process Security and Access Rights, please visit https://docs.microsoft.com/en-us/windows/win32/procthread/process-security-and-access-rights.

#### Configuration

##### Whitelist

You can whitelist an entire folder or a simple executable if you don't want the driver to filter these files.  
Modify the registry key ```HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender\Parameters\Whitelist```, then add folders and applications.

##### Cache

The cache holds by default 100 hashes corresponding to the last 100 applications that were analyzed.
It allows the driver to cache recurrent files and save computing time.  
Modify the registry key ```HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender\Parameters\CacheSize``` to increase or decrease the number of hashes hold by the cache.

##### Communication Port

The communication port is used by the driver and the user-mode service to communicate through a specified channel.
By default, the communication port is labelled _\PIDefenderPort_.  
Modify the registry key ```HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender\Parameters\CommunicationPort``` to modify the communication port.

##### Number of clients

By default, the number of clients is set to 1.  
Modify the registry key ```HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender\Parameters\MaxClients``` to increase this number.

### Service User-Mode

#### Verify Signature

There are two kinds of signature in Windows:
* Embedded signature.
* Catalog signature.

Both are checked in PI-Defender in order to trust an application.

#### Configuration

##### Listener Thread

The listener thread purpose is to wait for data send by the driver. Once a data is received it create a work pool and send the data to the worker thread.

Modify the registry key ```HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender_UM\Parameters\ListenerThreads``` to increase this number.

##### Workers Threads

Workers threads have multiple goals:
* Verify the signature of an executable.
* Send the response back to the driver.
* Push a new message in queue for the listener.

Modify the registry key ```HKLM\SYSTEM\CurrentControlSet\Services\PI-Defender_UM\Parameters\MaxWorkerThreads \ MinWorkerThreads``` to increase/decrease the number of workers threads.

## Maintainers/Authors

* Nicolas JALLET (@Nikj-Fr)
* Bérenger BRAULT (@CapitaineHadd0ck)

## Credits

* [NAVAL-Group](https://www.naval-group.com/en) for the apprenticeship and for accepting to release the tool in Open-Source.
* Baptiste DAVID

## Licensing

PI-Defender is under LPGLv3 licensed. Please refer to https://choosealicense.com/licenses/lgpl-3.0 for detailled information.

