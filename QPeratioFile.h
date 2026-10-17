#pragma once
# include "GroupMember.h"
# include "ChatMsg.h"
# include "CCMianWidget.h"

# include <QWidget>
# include <QFile>
# include <QReadWriteLock>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
# include <QSettings >

# define FILENAME_IMAGE "Image" //	成员头像保存路径
# define FILENAME_JSON  "JSON"	// 缓存文件保存路径
# define FILENAME_JSONIMAGE "JSON_Iamge"//	缓存图像文件

class QPeratioFile : public QObject
{
	Q_OBJECT
public:
	QPeratioFile(QObject* parent = nullptr);
	~QPeratioFile();

	//写入图像文件
	void WriteFileImage(const QString& namefile, const QByteArray& imagepkg);
	QString WriteFileImage(const QString& FileUrl, QString FileName, const QByteArray& imagePkg);
	QPixmap getImage(const QString& Image_Name);	//拿取图像文件
	QString getImageURL(const QString& AccountId);	//拿取图像文件路径
    QPixmap getImage(const QString& URL, const QString& Name);  //拿取缓存文件中的图像

	//void WriteFileJSON();							//写入成员对象JSON文件
	void FileSaveURL();								//文件保存的路径
	void FileSaveJSON();							//JSON文件保存路径
	void FileSaveJSON_Image();						//聊天图像数据缓存文件
    void CreateFileJson(QString& filrURL);          //创建JSON文件


	void initFile();
	static QPeratioFile* instance();						//返回全局单列文件对象

	// ===================== 对外业务接口 =====================
   // 保存指定群的全部成员到JSON
	bool saveGroupMemberToJson(const QString& GroupId, const QList<GroupMenber>& memberList);
	bool saveGroupMemberToJson(const QMap<QString, GroupMenber>& Map, const QString& m_FileJsonGroupIdUrl);
	//读取单个群缓存成员列表
	QList<GroupMenber>loadGroupMemberFromJson(const QString& GroupId);
	//加载全部群缓存数据
	QMap<QString, QList<GroupMenber>>loadAllGroupMember();
	QMap<QString, GroupMenber>loadAllGroupMember(const QString& URL);//加载指定的群聊数据缓存
	//判断当前JSON缓存中是否存在该群号
	bool isGroupCacheExist(const QString& groupId);

	bool JudgmnetFile(const QString& fileName, const QString& uid);	//判断是否存在该文件
	QString NewFileURL(const QString& fileName, const QString& uid);	//新建该文件
	QString NewFileURL(const QString& FileName);		//新建文件夹

	//写入聊天记录的缓存数据
	bool SwitchFile(const QString& uid, const QList<ChatMsg>& list);
	//拿取聊天数据的缓存
	QList<ChatMsg> TakeMessageFile(const QString& uid);

    //写入联系人分组
    bool SwitchContactGrouping(const QMap<QString, QList<QString>>&, const QString&);
    //读取联系人分组
    void readJsonContactGriuping(QMap<QString, QList<QString>>& map, const QString strURl);

    QString getJsonFileURL();           //返回Json成员数据缓存文件的路径
    QString getImageFileURL();          //返回Image图像数据缓存文件的路径
    QString getJSONImageURL();          //返回聊天图像信息缓存文件的路径
    QString getJsonGroupFileURL();      //返回Json群信息缓存文件的路径
    QString getJsonContactFilURL()const;// 返回联系人分组缓存文件的路径
    QString getServerConfiguration ()const; //拿取服务器的配置文件路径

	//消息结构ChatMsg数据转换成QByteArrey
	QByteArray ChatMsgToByteArrey(const ChatMsg& msg);
	//QByteArrey数据转换为消息结ChatMsg构体数据
	ChatMsg QbyteArreyToChatMsg(const QByteArray& arr);

	//消息结构体数据转换为QVariantMap
	QVariantMap ChatMsgToQVariantMap(const ChatMsg& msg);
	//QVariantMap 转换为 ChatMsg
	ChatMsg QVariantMapToChatMsg(const QVariantMap& params);

    // ===================== 外部工具函数 =====================
    //生成毫秒加本次程序运行自增加的UID
    QString generateFilePath();
    // 缩放压缩图像数据
    QByteArray compressAndSendImage(const QString& imagePath,
                                    const QSize& targetSize,
                                    int quaity = 80);

private:
	// ===================== 数据信息内部转换工具函数 =====================
	//成员结构体转换为JSON 对象
	QJsonObject memberToJsonObj(const GroupMenber&);
	//JSON 对象转换为成员结构体
	GroupMenber jsonObjToMember(const QJsonObject&);

	// ===================== 聊天数据内部转换工具函数 =====================
	// 聊天 结构数据转换为 JSON对象
	QJsonObject MessageToJsonObj(const ChatMsg& );
	//JSON 对象转换为消息结构体
	ChatMsg jsonObjToMessage(const QJsonObject&);

	QJsonDocument readJsonDoc();// 读取整个JSON文档
	QJsonDocument readJsonDoc(const QString& URL);// 读取整个JSON文档
	//将JSON文档写入本地文件
	bool writeJsonDoc(const QJsonDocument& doc);
	bool writeJsonDoc(const QJsonDocument& doc, const QString& URL);//写入并创建指定的JSON文档

	//将图像数转换为2进制
	QByteArray pixmapToBytes(const QPixmap& pix);
private:
	
	QString m_FileURL;				//	文件保存的路径
	QString m_FileImageUrl;			//图像文件的 保存路径
	QString m_FileJSONUrl;			//群成员信息 JSON文件的 保存路径
	QString m_FileJsonGroupIdUrl;	//群信息 JSON文件的 保存路径
	QString m_FileJSONImageUrl;		//聊天图像表情缓存路径
    QString m_FileJsonContactURL;   //联系分组缓存保存路径
    QString m_FileServerConfiguration; //服务器ip端口配置文件保存路径

    //QMap<QString, QString> m_NameUrl;// 存储的头像路径与名字的映射

	static QReadWriteLock s_FileLock; //全局静态读写锁
    quint64 m_counter = 0;   //文件命名自增的UID ,单次运行内自增

};

