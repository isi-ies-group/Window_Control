# Contribution guide #

To work within the shared repository, follow the guidelines below:

#### 1. Clone the repository

> *git clone <repository_url>*
> *cd <repository_name>*

#### 2. Create a folder for your project

Each project must be placed in its own directory at the root of the repository:

> */esp32-base*
>
> */stm32-ui*
>
> */stm32-communication*

#### 3. Create a main branch for your project

Create a dedicated branch for your project starting from main:

> *git branch <directory_name>*
>
> *git checkout <directory_name>*

#### 4. Development workflow

For each new feature or modification:

Create a temporary working branch from your project branch:

> *git branch <local_branch_name>*
>
> *git checkout <local_branch_name>*

Develop your changes and commit regularly.

#### 5. Merge changes into your project branch

Once the feature is complete:

> *git checkout <directory_name>*
>
> *git merge <directory_name>*

After merging, delete the temporary branch:

> *git branch -d <local_branch_name>*

#### 6. Remote updates

**Only the main branch** of each project **should be pushed and maintained in the remote repository**.

**The main branch is reserved for stable versions or final integrations**.

#### 7. Version control

Each project must follow its own versioning scheme using standard semantic versioning:

Examples:

> _esp32-base-V1.0.0_
>
> _stm32-ui-V0.1.4_

Version numbers should be updated according to the scope of changes:

> _Major version → significant changes_
>
> _Minor version → new features_
>
> _Patch version → bug fixes_

### Who do I talk to? ###

* Repo owner
* ISI Team

### Funding ###

* Project supported by grant SMARTWIN TED2021-130920B-C21 funded by MCIN/AEI/10.13039/501100011033 and by the “European Union NextGenerationEU/PRTR”
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/989816c9-557e-42d0-b824-13ecb4693869" />
* Supported by the project MICROBEAM ref. PID2021-127810OB-I00, funded by MCIN/AEI/10.13039/501100011033  “ERDF A way of making Europe”
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/826b3fa0-f6d5-4a54-92b8-6f444c76ebec" />
* Esta actuación ha sido financiada mediante el programa de actividades de I+D con referencia TEC-2024/ECO-72 y acrónimo 4EVERPV-CM concedido por la Comunidad de Madrid a través de la Dirección General de Investigación e Innovación Tecnológica a través de la Orden 3177/2024.

<img width="194" height="260" alt="image" src="https://github.com/user-attachments/assets/6afbcb7b-0ccd-4889-b737-5a2f03837ba9" /> <img height="260" alt="image" src="https://github.com/user-attachments/assets/51639f79-adf4-435d-bda8-9c334cea3339" />




