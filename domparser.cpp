
#include "domparser.h"

#include <QRegularExpression>

#include "mainwindow.h"
extern MainWindow* mw_one;
extern bool binPlistFile;

QString strRootType;

DomParser::DomParser() {}
DomParser::~DomParser() {}

DomModel* DomParser::fromDom(QDomDocument d) {
  // get root element
  QDomElement root = d.documentElement();

  // check cosistency

  if (root.tagName() != "plist") {
    qWarning("The file is not a plist!");
    return NULL;
  }

  // get dict element
  QDomElement dict = root.firstChildElement();

  // create model
  DomModel* m = new DomModel();

  // create root item in model
  DomItem* item = m->getRoot()->addChild();
  item->setName("plist");
  item->setType(dict.tagName());

  // parse doc recursively
  parseElement(dict, item);

  return m;
}

QDomDocument DomParser::toDom(DomModel* m) {
  // create doc with doctype
  QDomDocument doc(
      "plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
      "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"");

  // create and add processing instruction
  QDomProcessingInstruction instr = doc.createProcessingInstruction(
      "xml", "version=\"1.0\" encoding=\"UTF-8\"");
  doc.appendChild(instr);

  // create root item in doc
  QDomElement doc_root = doc.createElement("plist");

  QDomAttr version = doc.createAttribute("version");
  version.setValue("1.0");

  doc_root.setAttributeNode(version);

  DomItem* item = m->itemForIndex(m->index(0, 0));
  strRootType = item->getType();
  QDomElement doc_dict = doc.createElement(strRootType);

  doc_root.appendChild(doc_dict);

  // add root item to doc
  doc.appendChild(doc_root);

  // get root item from model
  DomItem* model_root = m->getRoot()->child(0);

  // parse model recursively
  parseItem(model_root, doc_dict, doc, true);

  return doc;
}

void DomParser::parseElement(QDomElement& n, DomItem* item) {
  if (n.hasChildNodes()) {
    QDomElement element = n.firstChildElement();

    int nC = 1;

    while (!element.isNull()) {
      QString tagName = element.tagName();

      if (tagName == "key" || tagName == "array" || tagName == "dict" ||
          tagName == "string" || tagName == "integer" || tagName == "real" ||
          tagName == "data" || tagName == "date" || tagName == "bool" ||
          tagName == "true" || tagName == "false") {
        // create child
        DomItem* domItem = item->addChild();

        if (tagName == "key") {
          // get key value
          QString eName = element.firstChild().nodeValue();

          // get val element
          QDomElement val = element.nextSiblingElement();

          // get type and val
          QString eType = val.tagName();

          QString eValue = val.firstChild().nodeValue();

          if (eType == "false") {
            eType = "bool";
            eValue = "false";
          }

          if (eType == "true") {
            eType = "bool";
            eValue = "true";
          }

          if (eType == "data") {
            QByteArray bytes = QByteArray::fromBase64(eValue.toUtf8());
            eValue = ByteToHexStr(bytes);
          }

          /*if (eType == "real") {
            QString a = eValue;
            if (binPlistFile && a.length() > 15) {
              QString a1;
              for (int k = 0; k < a.length(); k++) {
                if (a.mid(k, 1) == "9" && a.mid(k + 1, 1) == "9" &&
                    a.mid(k + 2, 1) == "9" && a.mid(k + 3, 1) == "9" &&
                    a.mid(k + 4, 1) == "9") {
                  a1 = a.mid(0, k);
                  int b = a1.mid(a1.length() - 1, 1).toInt();
                  QString b1 =
                      a1.mid(0, a1.length() - 1) + QString::number(b + 1);
                  a = b1;
                  break;
                }
              }

              for (int k = 0; k < a.length(); k++) {
                if (a.mid(k, 1) == "0" && a.mid(k + 1, 1) == "0") {
                  a1 = a.mid(0, k);
                  a = a1;
                  break;
                }
              }
            }

            eValue = a;
          }*/

          // set data
          domItem->setData(eName, eType, eValue);

          parseElement(val, domItem);

          element = val.nextSiblingElement();
        } else {
          domItem->setType(tagName);

          //新增：数组类型key为空的情况
          QString eName = element.firstChild().nodeValue();
          QString eType = element.tagName();

          QString eValue = element.firstChild().nodeValue();
          if (eType == "data") {
            QByteArray bytes = QByteArray::fromBase64(eValue.toUtf8());
            eValue = ByteToHexStr(bytes);
          }

          if (eType == "true" || eType == "false") {
            eValue = eType;
            eType = "bool";
          }
          domItem->setData(eName, eType, eValue);

          domItem->setName(QString("Item %1").arg(nC++));
          // domItem->setName(QString::number(nC ++));

          parseElement(element, domItem);
          element = element.nextSiblingElement();
        }
      } else {
        qWarning("Skipping unrecognized element: %s", qPrintable(tagName));
        element = element.nextSiblingElement();
        continue;
      }
    }
  }
}

void DomParser::parseItem(DomItem* item, QDomElement& n, QDomDocument& doc, bool isRoot) {
  QDomElement element = n;
  if (!isRoot) {
    const QString type = item->getType();
    QString value = item->getValue();
    // Keys belong to dictionaries; array labels are display-only.
    if (n.tagName() == "dict") {
      QDomElement key = doc.createElement("key");
      key.appendChild(doc.createTextNode(item->getName()));
      n.appendChild(key);
    }
    element = doc.createElement(type == "bool" ? value.trimmed() : type);
    n.appendChild(element);
    if (type != "array" && type != "dict" && type != "bool") {
      if (type == "data") {
        value.remove(QRegularExpression("\\s"));
        value = QString::fromLatin1(HexStrToByte(value).toBase64());
      }
      element.appendChild(doc.createTextNode(value));
    }
  }
  for (int i = 0; i < item->childCount(); ++i)
    parseItem(item->child(i), element, doc);
}

QString DomParser::ByteToHexStr(QByteArray ba) {
  QString str = ba.toHex().toUpper();

  return str;
}

QByteArray DomParser::HexStrToByte(QString value) {
  return QByteArray::fromHex(value.toUtf8());
}
