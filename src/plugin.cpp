#include "plugin.h"

void CutieDesktopFileParserPlugin::registerTypes(const char *uri)
{
    qmlRegisterSingletonType<CutieDesktopFileParser>(
        uri, 1, 0, "CutieDesktopFileParser",
        &CutieDesktopFileParser::provider);

    qmlRegisterUncreatableType<DesktopEntryModel>(
        uri, 1, 0, "DesktopEntryModel",
        "DesktopEntryModel is created by CutieDesktopFileParser");

    qmlRegisterType<AppFilterProxyModel>(
        uri, 1, 0, "AppFilterProxyModel");
}