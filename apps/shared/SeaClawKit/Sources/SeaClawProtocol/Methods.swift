import Foundation

/// RPC method name constants for the SeaClaw gateway control protocol.
public enum Methods {
    public static let connect = "connect"
    public static let health = "health"
    public static let configGet = "config.get"
    public static let configSchema = "config.schema"
    public static let capabilities = "capabilities"
    public static let chatSend = "chat.send"
    public static let chatHistory = "chat.history"
    public static let chatAbort = "chat.abort"
    public static let configSet = "config.set"
    public static let configApply = "config.apply"
    public static let sessionsList = "sessions.list"
    public static let sessionsPatch = "sessions.patch"
    public static let sessionsDelete = "sessions.delete"
    public static let toolsCatalog = "tools.catalog"
    public static let channelsStatus = "channels.status"
    public static let cronList = "cron.list"
    public static let cronAdd = "cron.add"
    public static let cronRemove = "cron.remove"
    public static let cronRun = "cron.run"
    public static let skillsList = "skills.list"
    public static let skillsEnable = "skills.enable"
    public static let skillsDisable = "skills.disable"
    public static let updateCheck = "update.check"
    public static let updateRun = "update.run"
    public static let execApprovalResolve = "exec.approval.resolve"
    public static let usageSummary = "usage.summary"

    /// All supported method names.
    public static let all: [String] = [
        connect, health, configGet, configSchema, capabilities,
        chatSend, chatHistory, chatAbort,
        configSet, configApply, sessionsList, sessionsPatch, sessionsDelete,
        toolsCatalog, channelsStatus,
        cronList, cronAdd, cronRemove, cronRun,
        skillsList, skillsEnable, skillsDisable,
        updateCheck, updateRun,
        execApprovalResolve, usageSummary
    ]
}
