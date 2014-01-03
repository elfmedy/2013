//slider function
function sliderDate(value) {
    var month = value.getMonth() + 1;
    var day = value.getDate();
    return "" + value.getFullYear() + "-" + (month < 10 ? "0" + month : month) + "-" + (day < 10 ? "0" + day : day);
}

function sliderTime(value) {
    var temp1 = Math.floor(value / 60);
    var temp2 = Math.floor(value % 60);
    var hour = temp1 < 10 ? "0" + temp1 : "" + temp1;
    var second = "";
    if (temp2 < 15) second = "00";
    else if (temp2 < 30) second = "15";
    else if (temp2 < 45) second = "30";
    else second = "45";
    return hour + ":" + second;
}

function displayValues(slider, values) {
    if (values.min instanceof Date) {
        $('#timeResult_0').text(sliderDate(values.min));
        $('#timeResult_2').text(sliderDate(values.max));
    }
    else {
        $('#timeResult_1').text(sliderTime(values.min));
        $('#timeResult_3').text(sliderTime(values.max));
    }
}

$(document).ready(function() {
    //slider
    var now = new Date();
    var nowYear = now.getFullYear();
    var nowMonth = now.getMonth();
    var nowDay = now.getDate();
    var lastNow = new Date();
    lastNow.setTime(now.getTime() - 7 * 24 * 3600 * 1000);
    var lastestNow = new Date();
    lastestNow.setTime(now.getTime() - 90 * 24 * 3600 * 1000);
    var s1 = $("#sDate").dateRangeSlider({
        defaultValues: { min: new Date(lastestNow.getFullYear(), lastestNow.getMonth(), lastestNow.getDate()), max: new Date(nowYear, nowMonth, nowDay) },
        bounds: { min: new Date(lastestNow.getFullYear(), lastestNow.getMonth(), lastestNow.getDate()), max: new Date(nowYear, nowMonth, nowDay) },
        wheelMode: null,
        wheelSpeed: 8,
        arrows: true,
        valueLabels: "show",
        formatter: function(value) {
            var month = value.getMonth() + 1;
            var day = value.getDate();
            return "" + value.getFullYear() + "-" + (month < 10 ? "0" + month : month) + "-" + (day < 10 ? "0" + day : day);
        },
        durationIn: 0,
        durationOut: 400,
        delayOut: 200,
        range: { min: false, max: false }
    }).bind("valuesChanged", function(event, ui) { displayValues(ui.label, ui.values); });
    displayValues(s1, s1.dateRangeSlider("values"));

    var s2 = $("#sTime").rangeSlider({
        defaultValues: { min: 0, max: 1439 },
        bounds: { min: 0, max: 1439 },
        wheelMode: null,
        wheelSpeed: 8,
        arrows: true,
        valueLabels: "show",
        formatter: function(value) {
            var temp1 = Math.floor(value / 60);
            var temp2 = Math.floor(value % 60);
            var hour = temp1 < 10 ? "0" + temp1 : "" + temp1;
            var second = "";
            if (temp2 < 15) second = "00";
            else if (temp2 < 30) second = "15";
            else if (temp2 < 45) second = "30";
            else second = "45";
            if (hour == 23 && temp2 == 59) second = 59;
            return hour + ":" + second;
        },
        durationIn: 0,
        durationOut: 400,
        delayOut: 200,
        range: { min: false, max: false }
    }).bind("valuesChanged", function(event, ui) { displayValues(ui.label, ui.values); });
    displayValues(s2, s2.rangeSlider("values"));

    //checkBox
    $('#posCheckList').hcheckbox();
    $('#selectCheckList').hcheckbox();
    $('#peoColorCheck').hcheckbox();
    //-position
    $('#posCheckList :checkbox+label').each(function() {
        $(this).click(function() {
            var checkedValues = new Array();
            $('#posCheckList :checkbox').each(function() {
                if ($(this).is(':checked')) {
                    checkedValues.push($(this).next().text());
                }
            });
            $('#posResult').text(checkedValues.join(' + '));
        });
    });
    //-type
    $('#selectCheckList :checkbox+label').each(function() {
        $(this).click(function() {
            var checkedValues = new Array();
            $('#selectCheckList :checkbox').each(function() {
                if ($(this).is(':checked')) {
                    checkedValues.push($(this).next().text());
                }
            });
            $('#typeResult').text(checkedValues.join(' + '));
        });
    });
    $('#selectCheckList :checkbox+label').click();  //选中所有
    //-single
    $('#peoColorCheck :checkbox+label').each(function() {
        $(this).click(function() {
            var pColorArray = new Array();
            $('#peoColorCheck :checkbox').each(function() {
                if ($(this).is(':checked')) {
                    pColorArray.push($(this).next().text());
                }
            });
            if (pColorArray.length > 0) {
                $('#singleResult_0_1').text('颜色: ' + pColorArray.join(' + ') + ';');
            }
            else {
                $('#singleResult_0_1').text("");
            }
        });
    });

    //submit
    $('#button_submit').click(function() {
        //时间
        var submitDateMin = $('#sDate').find('.ui-rangeSlider-leftLabel').text().replace(/-/g, '');
        var submitDateMax = $('#sDate').find('.ui-rangeSlider-rightLabel').text().replace(/-/g, '');
        var submitTimeMin = $('#sTime').find('.ui-rangeSlider-leftLabel').text().replace(':', '');
        var submitTimeMax = $('#sTime').find('.ui-rangeSlider-rightLabel').text().replace(':', '');
        var strTime = 'ts=' + submitDateMin + submitTimeMin + '&te=' + submitDateMax + submitTimeMax;
        //摄像头号
        var checkedValues_0 = new Array();
        var posAll = 1;
        $('#posCheckList :checkbox').each(function() {
            if ($(this).is(':checked')) {
                checkedValues_0.push($(this).val());
            } else {
                posAll = 0;
            }
        });
        var strPos;
        if (posAll == 0) {
            strPos = '&ps=' + checkedValues_0.join('+');
        } else {
            strPos = '&ps=all';
        }
        //类型
        var checkedValues_1 = new Array();
        var tpAll = 1;
        $('#selectCheckList :checkbox').each(function() {
            if ($(this).is(':checked')) {
                checkedValues_1.push($(this).val());
            } else {
                tpAll = 0;
            }
        });
        var strType;
        if (tpAll == 0) {
            strType = '&tp=' + checkedValues_1.join('+');
        } else {
            strType = '&tp=all';
        }
        //颜色
        var checkedValues_2 = new Array();
        var colorAll = 1;
        $('#peoColorCheck :checkbox').each(function() {
            if ($(this).is(':checked')) {
                checkedValues_2.push($(this).val());
            } else {
                colorAll = 0;
            }
        });
        var strColor;
        if (colorAll == 0) {
            strColor = '&cp=' + checkedValues_2.join('+');
        } else {
            strColor = '&cp=all';
        }
        //图片
        var strPic = "";
        var strPicArr = $('#peopleImage').attr('src').split('/');
        if (strPicArr.length > 0)
            var strPic = '&pp=' + strPicArr.pop();

        window.location.href = encodeURI("result.aspx?" + strTime + strPos + strType + strColor + strPic);
    });

    //ajax image upload
    $(".various").fancybox({
        openEffect: 'none',
        closeEffect: 'none',
        nextEffect: 'none',
        prevEffect: 'none',
        loop: false,
        width: '660px',
        autoSize: true,
        beforeClose: function() {
            var imgObj = $('.fancybox-iframe').contents().find('#cuttedImage');
            if (imgObj.length == 0) return;   //if not find, exit
            var imgSrc = 'tmpupload/s' + $('.fancybox-iframe').contents().find('#cuttedImage').text();
            var who = $('.fancybox-iframe').contents().find('#whois').text();
            var imgObj;
            imgObj = $('#peopleImage');
            $('#singleResult_0_2').text("");
            imgObj.css('display', 'none');
            imgObj.attr('src', imgSrc);

            $("<img/>").attr("src", imgObj.attr("src")).load(function() {   //不选区域直接关闭不触发onload函数的
                var maxImageSize = 300;
                var real_width = this.width;
                var real_height = this.height;
                var ele = imgObj;
                if (real_width > maxImageSize) {
                    ele.height(maxImageSize * 1.0 * real_height / real_width);
                    ele.width(maxImageSize);
                    real_height = ele.height();
                    real_width = ele.width();
                }
                if (real_height > maxImageSize) {
                    ele.width(maxImageSize * 1.0 * real_width / real_height);
                    ele.height(maxImageSize);
                }
                ele.css('display', 'block');
                $('#singleResult_0_2').text("图片: 有;");
            });
        }
    });

});