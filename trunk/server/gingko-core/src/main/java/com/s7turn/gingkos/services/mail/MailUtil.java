/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.s7turn.gingkos.services.mail;

import java.util.ArrayList;
import java.util.List;
import javax.mail.MessagingException;
import javax.mail.internet.MimeMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.JavaMailSenderImpl;
import org.springframework.mail.javamail.MimeMessageHelper;
import org.springframework.util.StringUtils;

/**
 *
 * @author leo
 */
public class MailUtil {

    protected JavaMailSender mailSender;
    protected String from;
//	private Map<String, Object> mailTemplateMap = new HashMap<String, Object>();
    private static final String EMAIL_ADDR_REGEXP = "^.*@.*[.].{2,}$";

    public String getFrom() {
        return from;
    }

    public void setFrom(String from) {
        this.from = from;
    }


    public void setMailSender(JavaMailSender mailSender) {
        this.mailSender = mailSender;
    }

//	private Map<String, Object> paraMap = new HashMap<String, Object>();
    public void sendMail( String[] toUsers,  String subject, String content) throws Exception {
        try {

            if (toUsers != null) {

                MimeMessage message = ((JavaMailSenderImpl) mailSender).createMimeMessage();
                MimeMessageHelper helper = new MimeMessageHelper(message, false, "UTF-8");
                if (!StringUtils.hasText(from)) {
                    helper.setFrom(from);
                } else {
                    helper.setFrom("formnull@123.com");
                }

                helper.setTo(toUsers);
               
                helper.setSubject(subject);
                helper.setText(content, true);
                mailSender.send(message);
            }

        } catch (MessagingException ex) {
            throw new Exception(ex.getMessage());
        }

    }

    public String[] checkEmailFormat(String[] toUsers, List<String> errorList) {
        if (toUsers == null) {
            return null;
        }
        List<String> successList = new ArrayList<String>();
        for (int i = 0; i < toUsers.length; i++) {
            if (toUsers[i] != null && toUsers[i].matches(EMAIL_ADDR_REGEXP)) {
                successList.add(toUsers[i]);
            } else {
                errorList.add(toUsers[i]);
            }
        }
        return (String[]) successList.toArray(new String[0]);
    }
}
