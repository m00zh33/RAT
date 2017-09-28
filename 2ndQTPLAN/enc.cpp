#include "enc.h"

QMutex o_seqmute;
quint8 o_seq1 = 1;
quint8 o_seq2 = 1;
QMutex i_seqmute;
quint8 i_seq1 = 1;
quint8 i_seq2 = 1;

void senc(QByteArray &data, quint16 start)
{
    for (quint16 i = start; i < data.count(); ++i)
    {
        data[i] = data.at(i) ^ (((i + 101) * 43) % 256);
    }
}

QByteArray mkdata(QByteArray data, quint8 slot)
{
    QByteArray ret;
    if (slot > 127 || slot < 1) return ret;
    QByteArray segdata, segdatac;
    quint16 segsize, check;
    bool goon = true;
    while (goon)
    {
        o_seqmute.lock();
        ret.append(QChar(o_seq1));
        ret.append(QChar(o_seq2));

        if (++o_seq2 > 250) {
            o_seq2 = 1;
            if (++o_seq1 > 250) {
                o_seq1 = 1;
            }
        }
        o_seqmute.unlock();
        if (data.size() > 1485) {
            segdata = data.left(1485);
            data = data.mid(1485);
        } else {
            segdata = data;
            data.clear();
            goon = false;
        }
        check = qChecksum(segdata,segdata.size());
        segdatac = qCompress(segdata,9);
        if (segdatac.size() < segdata.size()) {
            segdata = segdatac;
            ret.append(QChar(slot+127));
        } else {
            ret.append(QChar(slot));
        }
        segdatac.clear();
        segsize = segdata.size();
        segdata.prepend(QByteArray::fromRawData((char*)&segsize,2));
        segdata.append(QByteArray::fromRawData((char*)&check,2));
        senc(segdata,0);
        ret.append(segdata);
        //TODO: add to global seq retransmitter
    }
    return ret;
}

bool dcdata(QByteArray &data, QByteArray &ret, quint8 &slot)
{
    //lets get seq1 and seq2
    if (data.size() < 3) return false;
    quint8 p_seq1, p_seq2;
    p_seq1 = (quint8)data.at(0);
    if (p_seq1 > 250) {
        switch (p_seq1) {
            case 251: //ACK
            case 252: //RETRANSMIT
                p_seq1 = (quint8)data.at(1);
                p_seq2 = (quint8)data.at(2);
                data = data.mid(3);
                //TODO: check if packet exists
                break;
            default:
                //future expansions; just get the size and skip the packet
                quint16 skip = (* (quint16*)data.mid(2,2).data() );
                skip += 4;
                data = data.mid(skip);
                return true;
        }
        switch (p_seq1) {
            case 251: //ACK
                //TODO: ack
                return true; //conditional
                break;
            case 252: //RETRANSMIT
                ///TODO retransmit
                return true; //conditional
                break;
        }
    }
    if (data.size() < 8) return false; //incomplete
    p_seq2 = (quint8)data.at(1);
    if (p_seq1 < 1 || p_seq2 < 1 || p_seq2 > 250) {
        //something is messed up in the header
        //wtf do we do now?
        data.clear(); //maybe we should just clear the first 2 bytes?
        return false;
    }
    i_seqmute.lock();
    if (i_seq1 == 1 && i_seq2 == 1) {
        i_seq1 = p_seq1;
        i_seq2 = p_seq2 + 1;

    } else {
        if ((i_seq2 != p_seq2) || (i_seq1 != p_seq1)) {
            //data.clear(); //seq error
            //return false;
            i_seq2 = p_seq2;
            i_seq1 = p_seq1;
        }
        ++i_seq2;
    }
    if (i_seq2 > 250) {
        i_seq2 = 1;
        if (++i_seq1 > 250) {
            i_seq1 = 1;
        }
    }
    i_seqmute.unlock();

    //TODO: check if seq exists and valid

    slot = (quint8)data.at(2);
    if (slot == 0 || slot == 255) {
        data.clear(); //data corruption
        return false;
    }
    quint16 size, check;
    QByteArray datad;
    datad = data.mid(3);
    senc(datad, 0);
    size = (* (quint16*)datad.left(2).data() );
    if (size > 1485 || size == 0) {
        //data corruption
        data.clear();
        return false;
    }
    //SEQ1 SEQ2 SLOT SIZE SIZE -content- CHECK CHECK
    if ((size + 7) > data.size()) {
        //packet incomplete
        return false;
    }
    ret = datad.mid(2, size);
    check = (* (quint16*)datad.mid(size+2,2).data() );
    datad.clear();
    data = data.mid(size + 7);
    if (slot > 127) {
        slot -= 127;
        ret = qUncompress(ret);
    }
    if (check != qChecksum(ret.data(),ret.size())) {
        //check fail; retransmit?
        return false;
    }
    return true;
}

void rstinseq()
{
    i_seq1 = 1;
    i_seq2 = 1;
}
