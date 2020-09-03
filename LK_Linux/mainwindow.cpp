#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qpixmap.h>
#include <string>
#include "stdio.h"
#include "stdlib.h"
#include "winscard.h"
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <iomanip>
#include <QMessageBox>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinterInfo>
#include <QtPrintSupport/QPrintPreviewWidget>
#include <qpainter.h>
#include <QDate>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

namespace vars
{

long rv;

SCARDCONTEXT hContext;
SCARDHANDLE hCard;

unsigned long dwActiveProtocol, dwReaders, dwRecvLength;
unsigned int i;

unsigned char pbRecvBuffer [MAX_BUFFER_SIZE_EXTENDED];

unsigned char pbSelBuffer[16] = { 0x00, 0xA4, 0x04, 0x00, 0x0B, 0xF3, 0x81, 0x00, 0x00, 0x02, 0x53, 0x45, 0x52, 0x49, 0x44, 0x01 };
unsigned char ef_select[8] = { 0x00, 0xA4, 0x08, 0x00 };
unsigned char pbcitanje[5] = { 0x00, 0xB0 };
}

char* konekcija()
{   
    using namespace vars;

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &hContext);

    rv = SCardListReaders(hContext, nullptr, nullptr, &dwReaders);
    char * mszReaders = new char [MAX_BUFFER_SIZE] ();
    rv = SCardListReaders(hContext, nullptr, mszReaders, &dwReaders);

    if (rv != 0) {
        QMessageBox citac;
        citac.setText(pcsc_stringify_error(rv));
        citac.exec();
    }
    else {
        char *r = mszReaders;
        while (*r)
        {
            cout << mszReaders << endl;
            r += strlen(r) +1;
        }

        rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED , SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);

        if (rv != 0) {
            QMessageBox konekcija;
            konekcija.setText(pcsc_stringify_error(rv));
            konekcija.exec();
        }

        dwRecvLength = sizeof (pbRecvBuffer);
        rv = SCardTransmit(hCard, SCARD_PCI_T1, pbSelBuffer, sizeof (pbSelBuffer), nullptr, pbRecvBuffer, &dwRecvLength);

        if (rv != 0) {
            QMessageBox sctransmit;
            sctransmit.setText(pcsc_stringify_error(rv));
            sctransmit.exec();
            rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
        }
    }
    return mszReaders;
}

string* sortiranje (string data)
{
    std::stringstream pombuff;
    string sortirano;
    string sortdel;
    string* sort = new string [12];
    int x = 2;
    int y = data.length();
    int z = 0;
    while ( x < y ){
        if (data[x] == 0x06){
            x += 2;
            sortdel = pombuff.str();
            sortdel.erase(sortdel.length() - 1);
            sort[z] = sortdel;
            z += 1;
            pombuff.str("");
        }else{
            pombuff << data[x];
        }
        x++;
    }
    return sort;
}

char * conv (string s)
{   
    char* cvs = new char[s.length() + 1]();
    strcpy(cvs, s.c_str());
    return cvs;
}

string citanje(unsigned char data [], unsigned char read [])
{

    using namespace vars;

    ef_select [4] = data [0];
    ef_select [5] = data [1];
    ef_select [6] = data [2];
    ef_select [7] = data [3];

    pbcitanje [2] = read [0];
    pbcitanje [3] = read [1];
    pbcitanje [4] = 0xff;

    string pbrb ;

    dwRecvLength = sizeof (pbRecvBuffer);
    rv = SCardTransmit(hCard, SCARD_PCI_T1, ef_select, sizeof (ef_select), nullptr, pbRecvBuffer, &dwRecvLength);

    if (rv != 0) {
        QMessageBox select;
        select.setText(pcsc_stringify_error(rv));
        select.exec();
        rv = SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
    }
    else {

        pbcitanje[2] = pbRecvBuffer[2];
        pbcitanje[3] = pbRecvBuffer[3];

        std::stringstream ssa;
        for (int q = 2; q<4; q++)
        {
            ssa << pbcitanje[q];
        }
        string o = ssa.str();
        int ef_len = pbcitanje[3] | pbcitanje [2] << 8;

        std::stringstream izlaz;
        int ef_off = 6;
        int limit;
        dwRecvLength = sizeof (pbRecvBuffer);
        while (ef_off < ef_len) {
            limit = ef_len - ef_off;
            if ( limit > 0xff){
                limit = 0xff;
            }
            pbcitanje [2] = ef_off>>8;
            pbcitanje [3] = ef_off;
            pbcitanje [4] = limit;

            rv = SCardTransmit(hCard, SCARD_PCI_T1, pbcitanje, sizeof (pbcitanje), nullptr, pbRecvBuffer, &dwRecvLength);

            if (rv != 0){
                QMessageBox citanje;
                citanje.setText(pcsc_stringify_error(rv));
                citanje.exec();
            }
            ef_off += limit;

            for(i=0; i<dwRecvLength; i++){
                izlaz << pbRecvBuffer[i];
                //                printf("%02X ", pbRecvBuffer[i]);
            }
        }
        pbrb = izlaz.str();
        cout << ef_len;
    }
    return pbrb;
}

string eraseAllSubStr(std::string & mainStr, const std::string & toErase)
{
    size_t pos = std::string::npos;


    // Search for the substring in string in a loop untill nothing is found
    while ((pos  = mainStr.find(toErase) )!= std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
    return  mainStr;
}

string ReplaceString(std::string subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

QString formatiranje(string f)
{
f.insert(2, ".");
f.insert(5, ".");
QString fmt = &f[0];
return fmt;
}

void MainWindow::on_pushButton_clicked()
{
    unsigned char podok [4] = { 0x02, 0x0f, 0x02, 0x06 };
    unsigned char licpod[4] = { 0x02, 0x0f, 0x03, 0x06 };
    unsigned char adresa[4] = { 0x02, 0x0f, 0x04, 0x06 };
    unsigned char slika[4] = { 0x02, 0x0f, 0x06, 0x06 };
    unsigned char reading[3] = { 0x00, 0x00, 0x06 };
    string data;
    string datapod;
    string datalp;
    string datapreb;
    string datasl;

    ui->label_27->setText(konekcija());

    datapod = citanje(podok, reading);     //čitanje podataka o dokumentu
    string* flabels;
    flabels = sortiranje(datapod);
    ui->label_22->setText(conv(flabels[0]));
    ui->label_24->setText(formatiranje(conv(flabels[4])));
    ui->label_26->setText(formatiranje(conv(flabels[5])));
    ui->label_20->setText(conv(flabels[6]));

    datalp = citanje(licpod, reading);     // lični podaci
    datalp.append("\x06");
    string* flabelsl;
    flabelsl = sortiranje(datalp);
    std::stringstream mestor;
    mestor << flabelsl[5];
    mestor << ", ";
    mestor << flabelsl[6];
    mestor << ", ";
    mestor << flabelsl[7];
    ui->label_2->setText(conv(flabelsl[1]));
    ui->label_4->setText(conv(flabelsl[2]));
    ui->label_6->setText(conv(flabelsl[3]));
    ui->label_8->setText(formatiranje(conv(flabelsl[8])));
    ui->label_18->setText(conv(flabelsl[4]));
    ui->label_16->setText(conv(flabelsl[0]));
    ui->label_10->setText(conv(mestor.str()));

    datapreb = citanje(adresa, reading);     // prebivalište
    string* flabelsa;
    flabelsa = sortiranje(datapreb);
    stringstream prebiv;
    prebiv << flabelsa[2];
    prebiv << ", ";
    prebiv << flabelsa[1];
    prebiv << ", ";
    prebiv << flabelsa[3];
    prebiv << ", ";
    prebiv << flabelsa[4];
    prebiv << ", ";
    prebiv << flabelsa[5];
    // prebiv << ", ";
    ui->label_12->setText(conv(prebiv.str()));
    //    ui->label_4->setText(conv(flabelsa[2]));


    data = citanje(slika, reading);     //slika
    string sub ("\x90\0", 2);
    datasl = eraseAllSubStr(data, sub);
    datasl.erase(0,2);
    QByteArray dataslk = QByteArray::fromStdString(datasl);
    QPixmap mpixmap;
    mpixmap.loadFromData(dataslk,"JPG");
    ui->label_slika->setPixmap(mpixmap);
    cout << "Uspešno pročitana lična karta";
}

void MainWindow::print(QPrinter* prt)
{
    QDate datum = QDate::currentDate();
    QString dat = datum.toString("dd.MM.yyyy");
    QPainter p (prt);
    QPixmap slika = ui->label_slika->pixmap(Qt::ReturnByValue);
//  QPixmap slika = *ui->label_slika->pixmap();
    p.setRenderHints(QPainter::Antialiasing |
                     QPainter::TextAntialiasing |
                     QPainter::SmoothPixmapTransform, true);

    p.setFont({"Helvetica", 16});
    p.drawLine(50, 50, 780, 50);
    p.drawText(50, 70, "SLIKA : ");
    p.drawLine(50, 74, 780, 74);
    p.drawPixmap(50, 80, slika);
    p.drawLine(50, 450, 780, 450);
    p.drawText(50, 470, "PODACI O GRAĐANINU : ");
    p.drawLine(50, 474, 780, 474);
    p.setFont({"Helvetica", 12});
    p.drawText(50, 520, "Prezime : ");
    p.drawText(350, 520, ui->label_2->text());
    p.drawText(50, 540, "Ime : ");
    p.drawText(350, 540, ui->label_4->text());
    p.drawText(50, 560, "Ime oca : ");
    p.drawText(350, 560, ui->label_6->text());
    p.drawText(50, 580, "Datum rođenja : ");
    p.drawText(350, 580, ui->label_8->text());
    p.drawText(50, 600, "Mesto rođenja, opština i država : ");
    p.drawText(350, 600, ui->label_10->text());
    p.drawText(50, 620, "Prebivalište i adresa stana : ");
    p.drawText(350, 620, ui->label_12->text());
    p.drawText(50, 640, "Datum promene adrese : ");
    p.drawText(350, 640, ui->label_14->text());
    p.drawText(50, 660, "JMBG : ");
    p.drawText(350, 660, ui->label_16->text());
    p.drawText(50, 680, "Pol : ");
    p.drawText(350, 680, ui->label_18->text());
    p.setFont({"Helvetica", 16});
    p.drawLine(50, 710, 780, 710);
    p.drawText(50, 730, "PODACI O DOKUMENTU : ");
    p.drawLine(50, 734, 780, 734);
    p.setFont({"Helvetica", 12});
    p.drawText(50, 780, "Dokument izdaje : ");
    p.drawText(350, 780, ui->label_20->text());
    p.drawText(50, 800, "Broj dokumenta : ");
    p.drawText(350, 800, ui->label_22->text());
    p.drawText(50, 820, "Datum izdavanja : ");
    p.drawText(350, 820, ui->label_24->text());
    p.drawText(50, 840, "Važi do : ");
    p.drawText(350, 840, ui->label_26->text());
    p.drawLine(50, 870, 780, 870);
    p.drawText(50, 900, "Datum štampe : ");
    p.drawText(180, 900, dat);
    p.end();
}

void MainWindow::on_pushButton_2_clicked()
{
    QPrinter printer;
    printer.setOutputFormat(QPrinter::NativeFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);
    printer.setFullPage(true);
    printer.setResolution(100);
 //   printer->setOutputFileName("test.pdf");

    QPrintPreviewDialog *ppd = new QPrintPreviewDialog (&printer);
    ppd->setWindowTitle("Pregled Štampe");
    connect(ppd,SIGNAL(paintRequested(QPrinter*)),this,SLOT(print(QPrinter*)));
    ppd->exec();
}
