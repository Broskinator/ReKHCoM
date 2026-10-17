package com.rekhcom.android;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

/** Cartridge SRAM, independent of filename extension or host platform. */
public final class SaveImport {
    private SaveImport() {}
    public static String bankName(int region, int bank) {
        if (region < 0 || region > 2 || bank < 1 || bank > 5)
            throw new IllegalArgumentException("Región o banco inválido");
        String base = new String[]{"kh_com", "kh_com_us", "kh_com_jp"}[region];
        return base + (bank == 1 ? "" : "_bank" + bank) + ".sav";
    }
    public static byte[] normalize(byte[] source, int region) throws IOException {
        bankName(region, 1); // validate destination before parsing
        byte[] result;
        if (source.length == 32768) {
            result = Arrays.copyOf(source, 65536);
            Arrays.fill(result, 32768, result.length, (byte)0xff);
        } else if (source.length == 65536) {
            result = source.clone();
        } else if (source.length == 131072) {
            // Some SRAM containers repeat the same chip. Never discard a
            // different second bank or guess at undocumented wrapper headers.
            for (int n = 0; n < 65536; n++)
                if (source[n] != source[n + 65536])
                    throw new IOException("SRAM de 128 KiB ambigua; exporta una partida SRAM de 64 KiB");
            result = Arrays.copyOf(source, 65536);
        } else {
            throw new IOException("Se necesita SRAM .sav/.srm de 32 o 64 KiB, no un estado rápido");
        }
        byte[] wanted = ("KHCOM_BACKUP_VER0000001" + (region == 0 ? "3" : "2"))
                .getBytes(StandardCharsets.US_ASCII);
        int header = 0, other = 0;
        for (int n = 0; n < 2; n++) {
            int offset = 0x10 + n * 0x40;
            if (matches(result, offset, wanted) && checksum(result, offset, 0x40) == 0) header++;
            byte[] alternative = ("KHCOM_BACKUP_VER0000001" + (region == 0 ? "2" : "3"))
                    .getBytes(StandardCharsets.US_ASCII);
            if (matches(result, offset, alternative) && checksum(result, offset, 0x40) == 0) other++;
        }
        if (other != 0) throw new IOException("La partida no corresponde a esta región (Europa usa otro formato)");
        if (header == 0) throw new IOException("Cabecera de partida inválida o dañada");
        boolean playable = false;
        // Original save.c: system, two Sora files, two Riku files; each has
        // two redundant copies. Accept one healthy copy, as the game does.
        for (int n = 0; n < 2; n++)
            playable |= valid(result, 0x90 + n * 0xf14, 0xf14, wanted);
        for (int n = 0; n < 4; n++) {
            playable |= valid(result, 0x2f20 + n * 0xf14, 0xf14, wanted);
            playable |= valid(result, 0x1ec0 + n * 0x418, 0x418, wanted);
        }
        if (!playable) throw new IOException("No hay bloques de partida válidos; exporta el guardado normal del juego");
        return result;
    }
    private static boolean valid(byte[] data, int offset, int size, byte[] signature) {
        return matches(data, offset, signature) && checksum(data, offset, size) == 0;
    }
    private static boolean matches(byte[] data, int offset, byte[] signature) {
        for (int n = 0; n < signature.length; n++) if (data[offset + n] != signature[n]) return false;
        return true;
    }
    private static int checksum(byte[] data, int offset, int size) {
        long sum = 0;
        for (int n = offset; n < offset + size; n += 2)
            sum += (data[n] & 255) | ((data[n + 1] & 255) << 8);
        sum = (sum & 65535) + (sum >> 16);
        return (int)(~(sum + (sum >> 16))) & 65535;
    }
}
