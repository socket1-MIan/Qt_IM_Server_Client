#include "QPeratioFile.h"
# include <QCoreApplication>
# include <QDir>
# include <QImage>
# include <QBuffer>

// 创建全局静态对象,单列模式
Q_GLOBAL_STATIC(QPeratioFile, m_File);
//静态全局容器
QReadWriteLock QPeratioFile::s_FileLock;

QPeratioFile::QPeratioFile(QObject* parent):
	QObject(parent)
{
	initFile();			// 初始化需要创建的文件夹名称 
	FileSaveURL();		//创建存储数据的文件夹
	FileSaveJSON();		//创建存储JSON对象的文件夹
	FileSaveJSON_Image(); //创建聊天图像数据缓存文件夹
    CreateFileJson(m_FileJsonContactURL);// 创建联系人分组缓存文件
}

QPeratioFile::~QPeratioFile()
{
}

//写入图像文件
void QPeratioFile::WriteFileImage(const QString& namefile, const QByteArray& imagepkg)
{
	//写锁
	//s_FileLock.lockForWrite();

	const QString& FileName = m_FileImageUrl;
	QImage image;
	const QString& URL = FileName + "/" + namefile + ".png";
	
	

	QFile isFile(URL);
	if (!isFile.exists())
	{
		QFile file(FileName);
		if (!file.exists()) {
			qDebug() << "文件路径中不存在该文件目录";
			file.close();
			return;
		}
		file.close();
		if (!image.loadFromData(imagepkg)) {
			qDebug() << "二进制数据转换为img写入文件失败";
			return;
		}

		if (!image.save(URL, "JPEG", 100)) {
			qDebug() << "图像文件写入失败";

		}
		
	}
	
	//s_FileLock.unlock();

}

//写入图像文件  聊天缓存
QString QPeratioFile::WriteFileImage(const QString& FileUrl,  QString FileName, const QByteArray& imagePkg)
{
	if (FileUrl.isEmpty() || FileName.isEmpty() || imagePkg.isEmpty()) {
        qDebug()<< "写入聊天图像缓存数据时， 有数据为空";
		return  NULL;
	}
	FileName.replace(":", "");

	QString URL = FileUrl + "/" + FileName + ".png";
	QImage image;
	QFile file(URL);

	if (!file.exists()) {
		file.close();
		if (!image.loadFromData(imagePkg)) {
			qDebug() << " 聊天图像缓存数据， 图像2进制转换为图像.png失败";
			return NULL; 
		}
		if (!image.save(URL, "JPEG", 100)) {
			qDebug() << "保存为.png失败";
			return NULL; 
		}

		return URL; 
	}
	else {
		qDebug() << "聊天图像缓存文件夹中已有该图像文件";
		file.close();
		return NULL; 
	}


}

//JSON文件保存路径
void QPeratioFile::FileSaveJSON()
{
	const QString& filePath = QCoreApplication::applicationDirPath();
	const QString& fileName = filePath + "/" + m_FileJSONUrl;

	//创建JSON文件夹
	QDir dir(fileName);
	if (!dir.exists()) {
		if (dir.mkpath(".")) {
			qDebug() << "已经成功创建该JSON保存文件夹" << dir.absolutePath();
			m_FileJSONUrl = dir.absolutePath()+ "/" + "Group.json";
			m_FileJsonGroupIdUrl = dir.absolutePath()+ "/" + "GroupId.json";
		}
		else
			qDebug() << "该文件夹创建失败" << fileName;
	}
	else {
		m_FileJSONUrl = fileName + "/" + "Group.json";
		m_FileJsonGroupIdUrl = fileName + "/" + "GroupId.json";
	}
}

//聊天图像数据缓存文件
void QPeratioFile::FileSaveJSON_Image()
{
	QString FileUrl = QCoreApplication::applicationDirPath();
	QString FileName = FileUrl + "/" + m_FileJSONImageUrl;

	QDir dir(FileName);
	if (!dir.exists()) {
		if (dir.mkpath(".")) {
			qDebug() << "聊天图像数据缓存文件夹保存成功";
			m_FileJSONImageUrl = FileName;
		}
		else {
			qDebug()<< "聊天图像数据缓存文件夹保存成功";
		}
	}
	else {
		m_FileJSONImageUrl = FileName;
        qDebug() << "聊天图书数据缓存已经创建 创建路径" << FileName;
	}

}

//创建JSON文件
void QPeratioFile::CreateFileJson(QString &filrURL)
{
    //获取当前执行文件的路径
    const QString filePath = QCoreApplication::applicationDirPath();
    m_FileURL = filePath;
    const QString fileName = m_FileURL + "/" + filrURL;

    QDir dir(fileName);
    if(!dir.exists(".")){
        if (dir.mkpath(".")) {
            qDebug() << "已经成功创建该保存文件夹" << dir.absolutePath();
            filrURL = dir.absolutePath() + "/" + "Grouping.json";
        }
        else
            qDebug() << "该文件夹创建失败" << fileName;
    }else{

        filrURL = fileName + "/" + "Grouping.json";
        qDebug() <<"当前创建的文件夹路径" ;
    }

}

//文件保存的路径
void QPeratioFile::FileSaveURL()
{
	//获取当前执行文件的路径
	const QString& filePath = QCoreApplication::applicationDirPath();
	m_FileURL = filePath;
	const QString& fileName = m_FileURL +"/" +  m_FileImageUrl;
	//创建图像文件夹
	QDir dir(fileName);
	if (!dir.exists()) {
		if (dir.mkpath(".")) {
			qDebug() << "已经成功创建该图像保存文件夹" << dir.absolutePath();
			m_FileImageUrl = dir.absolutePath();
		}
		else
			qDebug() << "该文件夹创建失败" << fileName;
	}
	else
		m_FileImageUrl = fileName; 

}

//拿取图像文件
QPixmap QPeratioFile::getImage(const QString& Image_Name)
{
	//读锁
	//s_FileLock.lockForRead();
	const QString& URL = m_FileImageUrl + "/" + Image_Name + ".png";
	QPixmap pixmap(URL);
	if(pixmap.isNull()){
		qDebug() << "图像文件没有该文件";
		return pixmap;
	}
	//s_FileLock.unlock();
	return pixmap;
}

//拿取图像文件路径
QString QPeratioFile::getImageURL(const QString& AccountId)
{
	QString URL = m_FileImageUrl + "/" + AccountId + ".png";
	QPixmap pixmap(URL);
	if (pixmap.isNull()) {
		qDebug() << "保存的图像文件中没有该文件";
		return NULL;
	}

	return URL;
}

//拿取缓存文件中的图像
QPixmap QPeratioFile::getImage(const QString &URL, const QString &Name)
{
    const QString Path = URL + "/" + Name;
    QPixmap pixmap(Path);
    if(pixmap.isNull()){
        qDebug() << "保存的图像文件中没有该文件, path: " << Path;
        return pixmap;
    }
    return pixmap;
}

void QPeratioFile::initFile()
{
	m_FileImageUrl = "Image";
	m_FileJSONUrl = "JSON";
	m_FileJSONImageUrl = "JSON_Imager";
    m_FileJsonContactURL = "ContactGrouping";

    const QString filePath = QCoreApplication::applicationDirPath();

    m_FileServerConfiguration = filePath + "/" + "ServerConfiguration.ini";

}

//返回全局单列文件对象
QPeratioFile* QPeratioFile::instance()
{
	return m_File;
}

// ===================== 对外业务接口 =====================
// 保存指定群的全部成员到JSON
bool QPeratioFile::saveGroupMemberToJson(const QString& GroupId, const QList<GroupMenber>& memberList)
{
	QJsonDocument doc = readJsonDoc();
	QJsonObject rootObj = doc.object();

	// 构建当前群的成员数组
	QJsonArray memberArr;
	for (const GroupMenber& member : memberList) {
		memberArr.append(memberToJsonObj(member));
	}
	//根对象写入当前群数组,覆盖旧值
	rootObj[GroupId] = memberArr;

	//重新封装文档并写入文件
	doc.setObject(rootObj);

	return writeJsonDoc(doc);
}

//写入聊天记录的缓存数据
bool QPeratioFile::SwitchFile(const QString& uid, const QList<ChatMsg>& list)
{
	QString URL = uid + ".json";

	QString FilePath = NewFileURL("JSON", URL);
	if (!FilePath.isEmpty()) {

		QJsonDocument doc = readJsonDoc(FilePath);
		QJsonObject rootObj = doc.object();
		QJsonArray ary;
		for (const ChatMsg& member : list) {
			ary.append(MessageToJsonObj(member));
		}
		rootObj[uid] = ary;
		doc.setObject(rootObj);

		return writeJsonDoc(doc, FilePath);
	}
	else {
		qDebug() << "写入JSON聊天记录历史数据失败";
		return false;
	}

}
//拿取聊天数据的缓存
QList<ChatMsg> QPeratioFile::TakeMessageFile(const QString& uid)
{
	QList<ChatMsg> member;
	QString NameFile = uid + ".json";
	QString  FilePath = NewFileURL("JSON", NameFile);
	if (!FilePath.isEmpty()) {

		QJsonDocument doc = readJsonDoc(FilePath);
		QJsonObject rootObj = doc.object();
		QJsonArray arry = rootObj[uid].toArray();

		for (QJsonValue val : arry) {
			member.append(jsonObjToMessage(val.toObject()));
		}
		return member; 
	}
	else {
		return member;
	}
	
}

 //写入联系人分组
bool QPeratioFile::SwitchContactGrouping(const QMap<QString, QList<QString>> &map, const QString& strURL)
{

    QJsonDocument doc = readJsonDoc(strURL);
    QJsonObject rootObj = doc.object();

    auto iter = map.constBegin();
    for(; iter != map.constEnd(); iter++){

        QString ContactKey = iter.key();
        const QList<QString>& list = iter.value();

        QJsonArray arr;
        for(const QString& s : list){
            arr.append(s) ;
        }
        rootObj[ContactKey] = arr;
    }

    doc.setObject(rootObj);
    return writeJsonDoc(doc, strURL);
}

//读取联系人分组
void QPeratioFile::readJsonContactGriuping(QMap<QString, QList<QString>>& map, const QString strURl)
{

    QJsonDocument doc = readJsonDoc(strURl);
    QJsonObject Obj = doc.object();
    for(auto iter = Obj.begin(); iter != Obj.end(); iter++){

        QString key = iter.key();
        QJsonValue val = iter.value();
        if(!val.isArray()){
            continue;
        }

        QJsonArray arr = val.toArray();
        QList<QString> strList;
       //qDebug()<<"数组的长度: "<<arr.size();
        for(const QJsonValue& av : arr){

            if(av.isString()){
                //qDebug() <<"-- 数据测试 --- key : , value: " << key <<av.toString();
                strList.append(av.toString());
            }
        }

        map.insert(key, strList);
    }

}


//保存多个群聊信息到指定的Json文档中
bool QPeratioFile::saveGroupMemberToJson(const QMap<QString, GroupMenber>& Map, const QString& m_FileJsonGroupIdUrl)
{
	QJsonDocument doc = readJsonDoc(m_FileJsonGroupIdUrl);
	QJsonObject rootObj = doc.object();

	auto iter = Map.constBegin();
	for (; iter != Map.constEnd(); iter++) {

		 QString groupKey = iter.key();
		 GroupMenber groupValue = iter.value();
		// 群信息结构体转为JsonObject，覆盖/新增该群记录
		rootObj[groupKey] = memberToJsonObj(groupValue);
	}

	//重新封装文档并写入文件
	doc.setObject(rootObj);

	return writeJsonDoc(doc, m_FileJsonGroupIdUrl);
}


//读取单个群缓存成员列表
QList<GroupMenber> QPeratioFile::loadGroupMemberFromJson(const QString& GroupId)
{
	QList<GroupMenber> resList; 
	QJsonDocument doc = readJsonDoc();
	QJsonObject rootObj = doc.object();

	//判断是否存在该成员缓存
	if (!rootObj.contains(GroupId))
		return resList;

	//获取群对应的成员数组
	QJsonArray arr = rootObj[GroupId].toArray();
	for (const QJsonValue& val : arr) {
		QJsonObject memberObj = val.toObject();
		resList.append(jsonObjToMember(memberObj));
	}

	return resList;
}

//加载全部群缓存数据
QMap<QString, QList<GroupMenber>> QPeratioFile::loadAllGroupMember()
{
	QMap<QString, QList<GroupMenber>> allGroupData;
	QJsonDocument doc = readJsonDoc();
	QJsonObject rootObj = doc.object();

	//遍历根对象所有key(群号)
	QStringList groupIdList = rootObj.keys();
	for (const QString& gid : groupIdList) {

		QJsonArray arr = rootObj[gid].toArray();
		QList<GroupMenber> memberList;
		for (const QJsonValue& val : arr) {
			memberList.append(jsonObjToMember(val.toObject()));
		}
		allGroupData.insert(gid, memberList);
	}
	return allGroupData;
}

//加载指定的群聊数据缓存
QMap<QString, GroupMenber> QPeratioFile::loadAllGroupMember(const QString& URL)
{
	QMap<QString, GroupMenber> allGroupData;
	QJsonDocument doc = readJsonDoc(URL);
	QJsonObject rootObj = doc.object();
	//先遍历所有的群号
	 QStringList groupIdList = rootObj.keys();
	for (const QString& gid : groupIdList){

		QJsonValue val = rootObj.value(gid);
		// 强容错：校验该节点必须是JSON Object，跳过非法数据
		if (!val.isObject())
			continue;

		QJsonObject obj = val.toObject();
		// 禁止引用绑定临时返回值，使用值拷贝
		GroupMenber member = jsonObjToMember(obj);
		allGroupData.insert(gid, member);
	}

	return allGroupData;
}

//判断当前JSON缓存中是否存在该群号
bool QPeratioFile::isGroupCacheExist(const QString& groupId)
{
	QJsonDocument doc = readJsonDoc();
	QJsonObject rootObj = doc.object();

	return rootObj.contains(groupId);
}

//判断是否存在该文件
bool QPeratioFile::JudgmnetFile(const QString& fileName, const QString& uid)
{
	
	QString Path = QCoreApplication::applicationDirPath();
	QString filePath = Path + "/" + fileName + "/" + uid;
	QFile file(filePath);
	if (file.exists()) {
		file.close();
		return true;
	}
	else
	{
		file.close();
		return false;
	}

}

//新建该文件
QString QPeratioFile::NewFileURL(const QString& fileName, const QString& uid)
{
	QString Path = QCoreApplication::applicationDirPath();
	QString FilePath = Path + "/" + fileName + "/" + uid;
	if (!JudgmnetFile(fileName, uid)) {
		
		QFile file(FilePath);
		if (file.open(QFile::WriteOnly)) {
			file.close();
			return FilePath;
		}
		else {
			file.close();
			return NULL;
		}

	}
	return FilePath;
}

//新建文件夹
QString QPeratioFile::NewFileURL(const QString& FileName)
{
	QString File = QCoreApplication::applicationDirPath(); 
	QString Name = File + "/" + FileName;

	QDir dir(Name);
	if (!dir.exists()) {
		if (dir.mkpath(".")) {
			return Name;
			qDebug() << Name << "文件夹新建成功" << FileName;
		}
		else {
			qDebug() << " 文件夹新建失败";
			return NULL;
		}
		}
	else {
		qDebug() << "文件夹" << FileName << "新建成功";
		return Name; 
	}
}


//返回Json成员数据缓存文件的路径
QString QPeratioFile::getJsonFileURL()
{
	return m_FileJSONUrl;
}

//返回Image图像数据缓存文件的路径
QString QPeratioFile::getImageFileURL()
{
    return m_FileImageUrl;

}

//返回聊天图像信息缓存文件的路径
QString QPeratioFile::getJSONImageURL()
{
    return m_FileJSONImageUrl;
}

//返回Json群信息缓存文件的路径
QString QPeratioFile::getJsonGroupFileURL()
{
	return m_FileJsonGroupIdUrl;
}

// 返回联系人分组缓存文件的路径
QString QPeratioFile::getJsonContactFilURL()const
{
    return m_FileJsonContactURL;

}

//拿取服务器的配置文件路径
QString QPeratioFile::getServerConfiguration() const
{

    return m_FileServerConfiguration;
}

//消息结构ChatMsg数据转换成QByteArrey
QByteArray QPeratioFile::ChatMsgToByteArrey(const ChatMsg& msg)
{
	QJsonObject obj = MessageToJsonObj(msg);
	QJsonDocument doc(obj);
	// toJson() 直接输出UTF-8编码的字节数组
	return doc.toJson(QJsonDocument::Compact);
}

//QByteArrey数据转换为消息结ChatMsg构体数据
ChatMsg QPeratioFile::QbyteArreyToChatMsg(const QByteArray& arr)
{
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
	// 解析失败返回空
	if (err.error != QJsonParseError::NoError) {
		return ChatMsg();
	}
	return jsonObjToMessage(doc.object());

}

//消息结构体数据转换为QVariantMap
QVariantMap QPeratioFile::ChatMsgToQVariantMap(const ChatMsg& msg)
{

	QVariantMap params; 
	params["userName"] = msg.userName;		//发送者名称
	//params["avatarData"] = msg.avatarData;	//头像路径
	params["content"] = msg.content;		//聊天信息
	params["timeStr"] = msg.timeStr;		//发送的时间
    //params["strImageURL"] = msg.strImageURL;//头像的缓存路径
	params["isSelf"] = msg.isSelf;			//发送方的判断
	params["receive"] = msg.receive;		//接收对象
	params["send"] = msg.send;				//发送的对象
	params["msgType"] = msg.msgType;		//发送消息的类型
    params["resPath"] = msg.resPath;		//发送图像/表情信息的缓存

    if(msg.msgType == ChatMsg::MsgType::ImageMsg){

        QPixmap pixmap(msg.resPath);
        QByteArray pkg;
        if (!pixmap.isNull()) {
            //将要发送图像数据，缩放压缩
            pkg =compressAndSendImage(msg.resPath, QSize(100, 100), 85);
            if(pkg.isEmpty()){
                qDebug()<<"在发送图像数据时，图像压缩为JPEG2进制数据失败";
                pixmap.load(":/Resources/MainWindow/girl.png");
                pkg = pixmapToBytes(pixmap);
            }
        }
        else {//使用默认头像
             qDebug()<<"在发送图像数据时，头像路径不存在使用默认";
            pixmap.load(":/Resources/MainWindow/girl.png");
            pkg = pixmapToBytes(pixmap);
        }
        params["resPath"] = pkg;
    }
	return params;
}

//QVariantMap 转换为 ChatMsg
ChatMsg QPeratioFile::QVariantMapToChatMsg(const QVariantMap& params)
{
	ChatMsg msg;
    msg.userName = params["userName"].toString();		//发送者手机号
	
	//头像路径
	//先查看本地是否保存了当前头像缓存
	QString strURL = CCMianWidget::JudgmentContactImage(params["send"].toString());
	if (!strURL.isEmpty()) {
		msg.avatarData = strURL;
	}
	else {
        //如果没有保存该文件路径缓存
        //使用默认得头像路径
        msg.avatarData = ":/Resources/MainWindow/girl.png";
		
	}

	msg.content = params["content"].toString();			//聊天信息
	msg.timeStr = params["timeStr"].toString();			//发送的时间
    //msg.strImageURL = params["strImageURL"].toString();//头像的缓存路径
	msg.isSelf = false;			//发送方的判断
	msg.receive = params["receive"].toString();		//接收者
	msg.send = params["send"].toString();				//发送者
	msg.msgType = params["msgType"].toInt();			//发送消息的类型

	if (msg.msgType != ChatMsg::MsgType::TextMsg) {

        //拿取不会重复的文件名
        const QString StrName = generateFilePath();

        WriteFileImage(getJSONImageURL(), StrName, params["resPath"].toByteArray());
        QPixmap pixmap = getImage(QPeratioFile::instance()->getJSONImageURL(),
                                  StrName);
        if(pixmap.isNull())
            qDebug()<<"拿取发送得图像失败";

            msg.resPath = QPeratioFile::instance()->getJSONImageURL() + "/" + StrName;

		//发送图像/表情信息的缓存
        //msg.resPath = WriteFileImage(m_FileJSONImageUrl, params["timeStr"].toString(), params["resPath"].toByteArray());
	}

	return msg;
}

//生成毫秒加本次程序运行自增加的UID
QString QPeratioFile::generateFilePath()
{

    // 纳秒时间戳 + 自增序号，双重保证
    qint64 ts = QDateTime::currentMSecsSinceEpoch();
    quint64 id = ++m_counter;

    QString fileName = QString("%1_%2").arg(ts).arg(id, 6, 10, QChar('0'));
    // 例如: 1728000000000_000001.png

    return fileName;
}

// 缩放压缩图像数据
QByteArray QPeratioFile::compressAndSendImage(const QString &imagePath, const QSize &targetSize, int quaity)
{
    QImage image(imagePath);
    if(image.isNull()) return QByteArray();

    //缩放(保持宽高比)
    QImage scaled = image.scaled(targetSize,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation );

    //压缩为JPEG
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    scaled.save(&buffer, "JPEG", quaity);
    buffer.close();

    return data;
}




//成员结构体转换为JSON 对象
QJsonObject QPeratioFile::memberToJsonObj(const GroupMenber& member)
{
	QJsonObject obj;
	obj["MobileNember"] = member.MobileNember;
	obj["QQNember"] = member.QQNember;
	obj["GroupId"] = member.GroupId;
	obj["Sign"] = member.Sign; 
	obj["URL"] = member.URL; 
 
	return obj;
}

//JSON 对象转换为成员结构体
GroupMenber QPeratioFile::jsonObjToMember(const QJsonObject& obj)
{
	GroupMenber member; 
	member.MobileNember = obj["MobileNember"].toString();
	member.QQNember = obj["QQNember"].toString();
	member.GroupId = obj["GroupId"].toString();
	member.Sign = obj["Sign"].toString(); 
	member.URL = obj["URL"].toString();

	return member;
}


//JSON 对象转换为消息结构体
ChatMsg QPeratioFile::jsonObjToMessage(const QJsonObject& obj)
{
	ChatMsg msg;
	msg.userName = obj["userName"].toString();		//发送者名称
	msg.avatarData = obj["avatarData"].toString();	//头像路径
	msg.content = obj["content"].toString();		//聊天信息
	msg.timeStr = obj["timeStr"].toString();		//发送的时间
    //msg.strImageURL = obj["strImageURL"].toString();//头像的缓存路径
	msg.isSelf = obj["isSelf"].toBool();			//发送方的判断
	msg.receive = obj["receive"].toString();		//接收者
	msg.send = obj["send"].toString();				//发送者
	msg.msgType = obj["msgType"].toInt();			//发送消息的类型
	msg.resPath = obj["resPath"].toString();		//发送图像/表情信息的缓存

	return msg;
}

// 聊天 结构数据转换为 JSON对象
QJsonObject QPeratioFile::MessageToJsonObj(const ChatMsg& member)
{
	QJsonObject obj; 

	obj["userName"] = member.userName;		//发送者名称
	obj["avatarData"] = member.avatarData;	//头像路径
	obj["content"] = member.content;		//聊天信息
	obj["timeStr"] = member.timeStr;		//发送的时间
    //obj["strImageURL"] = member.strImageURL;//头像的缓存路径
	obj["isSelf"] = member.isSelf;			//发送方的判断
	obj["receive"] = member.receive;		//接收对象
	obj["send"] = member.send;				//发送的对象
	obj["msgType"] = member.msgType;		//发送消息的类型
	obj["resPath"] = member.resPath;		//发送图像/表情信息的缓存


	return obj;
}


// 读取整个JSON文档
QJsonDocument QPeratioFile::readJsonDoc()
{
	if (m_FileJSONUrl.isEmpty()) {
		qDebug() << "当前的JSON路径不存在"; 
		return QJsonDocument(QJsonObject());
	}

	QFile file(m_FileJSONUrl);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		qDebug() << "读取群成员JSON 文件失败" << file.errorString();
		return QJsonDocument(QJsonObject());
	}
	//读取全部文本
	QByteArray jsonData = file.readAll();
	file.close();

	//解析文本为JSON文档
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
	if (err.error != QJsonParseError::NoError) {
		qDebug() << "JSON 解析错误" << err.errorString();
		return QJsonDocument(QJsonObject());
	}
	return doc; 
}

// 读取整个JSON文档
QJsonDocument QPeratioFile::readJsonDoc(const QString& URL)
{
	if (URL.isEmpty()) {
		qDebug() << "当前的JSON路径不存在";
		return QJsonDocument(QJsonObject());
	}

	QFile file(URL);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "读取群成员JSON 文件失败" << file.errorString();
		return QJsonDocument(QJsonObject());
	}
	//读取全部文本
	QByteArray jsonData = file.readAll();
	file.close();

	//解析文本为JSON文档
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
	if (err.error != QJsonParseError::NoError) {
		qDebug() << "JSON 解析错误" << err.errorString();
		return QJsonDocument(QJsonObject());
	}
	return doc;
}

//将JSON文档写入本地文件
bool QPeratioFile::writeJsonDoc(const QJsonDocument& doc)
{

	if (m_FileJSONUrl.isEmpty()) {
		//qDebug() << "当前的JSON路径不存在";
		//return false;
	}
	QFile file(m_FileJSONUrl);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "写入JSON文件失败:" << file.errorString();
		return false;
	}

	// 格式化输出（带换行缩进，方便人阅读，不加参数则压缩一行）
	QByteArray jsonText = doc.toJson(QJsonDocument::Indented);
	file.write(jsonText);
	file.close();

	return true; 
}

//写入并创建指定的JSON文档
bool QPeratioFile::writeJsonDoc(const QJsonDocument& doc, const QString& URL)
{
	if (m_FileJSONUrl.isEmpty()) {
		//qDebug() << "当前的JSON路径不存在";
		//return false;
	}
	QFile file(URL);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "写入JSON文件失败:" << file.errorString();
		return false;
	}

	// 格式化输出（带换行缩进，方便人阅读，不加参数则压缩一行）
	QByteArray jsonText = doc.toJson(QJsonDocument::Indented);
	file.write(jsonText);
	file.close();

	return true;
}

//将图像数转换为2进制
QByteArray QPeratioFile::pixmapToBytes(const QPixmap& pix)
{
	QByteArray ba;
	QBuffer buf(&ba);
	buf.open(QIODevice::WriteOnly);
	pix.save(&buf, "PNG"); // 可选 JPG、BMP
	buf.close();

	return ba;
}

