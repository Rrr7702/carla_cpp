#include <fbxsdk.h>

// 声明一个全局的 FbxManager 指针，初始化为 NULL，FbxManager 是 FBX SDK 中的核心管理类，
// 用于管理整个 FBX 相关的资源、场景等操作，这里先声明，后续应该会在合适地方进行初始化
// declare global
FbxManager*   gSdkManager = NULL;
// 以下分别声明用于不同物体的材质指针，它们都是FbxSurfacePhong类型，这种类型常用于表示具有Phong光照模型属性的材质，
// 不同的材质指针大概率会对应不同的模型部分，比如道路、人行道、十字路口、草地、街区等，方便后续为不同的场景元素设置独特的材质外观。
// materials
FbxSurfacePhong* gMatRoad;
FbxSurfacePhong* gMatSidewalk;
FbxSurfacePhong* gMatCross;
FbxSurfacePhong* gMatGrass;
FbxSurfacePhong* gMatBlock;

#ifdef IOS_REF
    #undef  IOS_REF
    #define IOS_REF (*(gSdkManager->GetIOSettings()))// 定义IOS_REF为gSdkManager的IO设置
#endif

// 创建一个材质并返回该材质
FbxSurfacePhong* CreateMaterial(FbxScene* pScene, char *name)
{
    // Create material
    // 使用 FbxSurfacePhong 类的静态方法 Create 为指定场景创建一个名为 name 的材质
    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, name);
    return lMaterial;// 返回创建的材质
}
// 查找字符串函数，检查name是否包含str
bool Find(const char *name, const char *str)
{
    // 获取输入字符串 name 和 str 的长度
    size_t lenName = strlen(name);
    size_t lenStr = strlen(str);
    // 如果长度不满足条件，返回 false
    if (lenName == 0 || lenStr == 0 || lenStr > lenName) return false;
    std::string strName(name);// 将name转换为std::string类型
    std::string strSub(str);// 将str转换为std::string类型
    // 在 strName 中查找 strSub，如果找到返回 true，否则返回 false
    return (strName.find(strSub)!= std::string::npos);
}
// 设置节点材质的函数
void SetMaterials(FbxNode* pNode)
{
    if (!pNode) return;// 将str转换为std::string类型
    // 初始化使用的材质为 gMatBlock
    FbxSurfacePhong* mat = gMatBlock;// 将str转换为std::string类型
    // 仅对网格节点进行操作
    FbxMesh* lMesh = pNode->GetMesh();
    if(lMesh)
    {
        // 移除节点上的所有材质
        pNode->RemoveAllMaterials();
        // 检查节点名称
        const char *name = pNode->GetName();
        if (Find(name, "Road_Road") || Find(name, "Roads_Road"))
            mat = gMatRoad;
        else if (Find(name, "Road_Marking") || Find(name, "Roads_Marking"))
            mat = gMatRoad;
        else if (Find(name, "Road_Curb") || Find(name, "Roads_Curb"))
            mat = gMatRoad;
        else if (Find(name, "Road_Gutter") || Find(name, "Roads_Gutter"))
            mat = gMatRoad;
        else if (Find(name, "Road_Sidewalk") || Find(name, "Roads_Sidewalk"))
            mat = gMatSidewalk;
        else if (Find(name, "Road_Crosswalk") || Find(name, "Roads_Crosswalk"))
            mat = gMatCross;
        else if (Find(name, "Road_Grass") || Find(name, "Roads_Grass"))
            mat = gMatGrass;
        // 打印节点名称和使用的材质名称
        printf("Node %s : %s\n", name, mat->GetName());
        // 为节点添加材质
        pNode->AddMaterial(mat);
    }
    // 递归遍历场景中的每个子节点并调用 SetMaterials 函数
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        SetMaterials(pNode->GetChild(i));
    }
}

// 从文件加载场景的函数
bool LoadScene(
               FbxManager* pSdkManager,  // Use this memory manager...
               FbxScene* pScene,            // to import into this scene
               const char* pFilename         // the data from this file.
               )
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    bool lStatus;
    // 获取当前使用的 FBX SDK 生成的 FBX 文件的版本号
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);
    // 创建一个导入器
    FbxImporter* lImporter = FbxImporter::Create(pSdkManager,"");
    // 初始化导入器，传入文件名和设置信息
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pSdkManager->GetIOSettings() );
    // 获取要导入的 FBX 文件的版本号
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
    if(!lImportStatus )  // Problem with the file to be imported
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        printf("Call to FbxImporter::Initialize() failed.");
        printf("Error returned: %s", error.Buffer());
        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            printf("FBX version number for this FBX SDK is %d.%d.%d",
                lSDKMajor, lSDKMinor, lSDKRevision);
            printf("FBX version number for file %s is %d.%d.%d",
                pFilename, lFileMajor, lFileMinor, lFileRevision);
        }
        return false;
    }
    if (lImporter->IsFBX())
    {
        // 设置导入时的属性，如不导入材质、纹理、链接等
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        false);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         false);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            false);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           false);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            false);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       false);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, false);
    }
    // 导入场景
    lStatus = lImporter->Import(pScene);
    // 销毁导入器
    lImporter->Destroy();
    return lStatus;
}

// Exports a scene to a file
// SaveScene函数用于将给定的FBX场景保存为指定格式的文件
// 它接受一系列参数来控制保存的相关设置，并根据操作结果返回成功（true）或失败（false）
// 参数说明如下：
// pSdkManager：指向FbxManager对象的指针，FbxManager是FBX SDK的核心管理类，用于管理整个FBX相关的资源、插件等操作，在这里作为保存场景操作的基础管理器，提供必要的上下文环境和功能支持。
// pScene：指向FbxScene对象的指针，代表了要保存的FBX场景，包含了场景中的所有模型、动画、材质等相关数据，是实际要保存到文件中的内容主体。
// pFilename：以字符串形式指定了要保存的文件路径及文件名，例如可以是"example.fbx"这样的形式，用于明确保存的目标位置和文件名称。
// pFileFormat：指定了保存文件时所采用的文件格式，不同的整数值对应不同的FBX支持的格式选项，通过这个参数可以控制生成的文件格式符合特定的需求。
// pEmbedMedia：布尔类型的参数，用于决定是否将相关的媒体资源（比如纹理图片等）嵌入到保存的FBX文件中，如果为true则嵌入，为false则不嵌入，具体取决于使用场景的需求。

// 将场景导出到文件的函数
bool SaveScene(
               FbxManager* pSdkManager,
               FbxScene* pScene,
               const char* pFilename,
               int pFileFormat,
               bool pEmbedMedia
               )
{
    bool lStatus = true;
    // 创建一个导出器
    FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");
    // 查找 OBJ 文件的写入器 ID
    pFileFormat = pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("Alias OBJ (*.obj)");
    // 初始化导出器，传入文件名、文件格式和设置信息
    if(lExporter->Initialize(pFilename, pFileFormat, pSdkManager->GetIOSettings()) == false)
    {
        printf("Call to FbxExporter::Initialize() failed.");
        printf("Error returned: %s", lExporter->GetStatus().GetErrorString());
        return false;
    }
    if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(pFileFormat))
    {
        // 设置导出时的属性，如导出材质、纹理等
        IOS_REF.SetBoolProp(EXP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(EXP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
        IOS_REF.SetBoolProp(EXP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(EXP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(EXP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
    }
    // 获取 FBX 场景的根节点
    FbxNode* lRootNode = pScene->GetRootNode();
    // 旋转根节点
    lRootNode->LclRotation.Set(FbxVector4(-90.0, 0.0, 0.0));
    // 根据 RoadRunner 的命名规则设置材质
    SetMaterials(lRootNode);
    // 导出场景
    lStatus = lExporter->Export(pScene);
    // 销毁导出器
    lExporter->Destroy();
    return lStatus;
}

int main(int argc, char **argv)
{
    // 创建一个 FBX SDK 管理器的实例
    gSdkManager = FbxManager::Create();
    // 创建一个 IOSettings 对象
    FbxIOSettings * ios = FbxIOSettings::Create(gSdkManager, IOSROOT );
    gSdkManager->SetIOSettings(ios);
    // 创建一个场景
    FbxScene* lScene = FbxScene::Create(gSdkManager,"");
    // 导入场景
    bool r = LoadScene(gSdkManager, lScene, argv[1]);
    if(!r)
    {
        printf("------- Import failed ----------------------------");
        // 销毁场景
        lScene->Destroy();
        return 0;
    }
    // 创建不同的材质
    gMatRoad     = CreateMaterial(lScene, "road");
    gMatSidewalk = CreateMaterial(lScene, "sidewalk");
    gMatCross    = CreateMaterial(lScene, "crosswalk");
    gMatGrass    = CreateMaterial(lScene, "grass");
    gMatBlock    = CreateMaterial(lScene, "block");
    // 导出场景
    r = SaveScene(gSdkManager, lScene, argv[2], -1, false);
    if(!r)
        printf("------- Export failed ----------------------------");
    // 销毁场景
    lScene->Destroy();
    // 销毁 FBX SDK 管理器
    if (gSdkManager) gSdkManager->Destroy();
    return 1;
}
